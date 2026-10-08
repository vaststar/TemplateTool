#pragma once

#include <ucf/services/UpgradeService/UpgradeModel.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class AsyncCallbackGate;

/// Downloads upgrade packages and verifies their integrity. All transport and
/// file cleanup work is serialized on the I/O worker; verification has its own
/// worker so a large hash cannot delay cancellation.
class UpgradeDownloadManager final
{
public:
    explicit UpgradeDownloadManager(
        ucf::framework::ICoreFrameworkWPtr coreFramework,
        std::shared_ptr<AsyncCallbackGate> callbackGate);
    ~UpgradeDownloadManager();

    UpgradeDownloadManager(const UpgradeDownloadManager&) = delete;
    UpgradeDownloadManager& operator=(const UpgradeDownloadManager&) = delete;

    using ProgressCallback = std::function<void(int64_t currentBytes, int64_t totalBytes)>;
    using DownloadCompleteCallback = std::function<void(
        bool success,
        const std::string& filePath,
        model::UpgradeErrorCode errorCode,
        const std::string& errorMessage)>;
    using VerifyCompleteCallback = std::function<void(
        bool success,
        model::UpgradeErrorCode errorCode,
        const std::string& errorMessage)>;

    void downloadPackage(const model::PackageInfo& packageInfo,
                         ProgressCallback progressCb,
                         DownloadCompleteCallback completeCb);
    void cancelDownload();
    void verifyPackage(const std::string& filePath,
                       const std::string& expectedSha256,
                       VerifyCompleteCallback callback);

    [[nodiscard]] std::filesystem::path getDownloadDirectory() const;
    [[nodiscard]] bool isDownloading() const;
    [[nodiscard]] bool hasSufficientSpace(int64_t requiredBytes) const;

    /// These only enqueue transport cancellation and file work. They are safe
    /// to call from an FSM transition while its mutex is held.
    void softReset();
    void hardReset();

    void setMaxRetryCount(int count);

    /// Shutdown is split so the owner can close the callback gate, stop all
    /// sub-managers, cancel requests, and then wait for workers and callbacks.
    void requestStop();
    void cancelOutstandingRequests();
    void joinWorkers();

private:
    struct DownloadSession
    {
        std::uint64_t generation{0};
        model::PackageInfo packageInfo{};
        ProgressCallback progressCb;
        DownloadCompleteCallback completeCb;

        // Mutable fields are protected by mSessionMutex.
        std::filesystem::path partialFilePath;
        bool resumeAllowed{false};
        std::int64_t resumedBytes{0};
        int currentRetry{0};
        std::chrono::steady_clock::time_point lastProgressTime{};
        std::string requestId;
    };

    using WorkerJob = std::function<void(std::stop_token)>;
    enum class IoJobKind { Cancel, Cleanup, Attempt };
    struct IoJob
    {
        IoJobKind kind;
        std::chrono::steady_clock::time_point due;
        std::uint64_t sequence;
        WorkerJob run;
    };

    void ensureDownloadDirectory();
    std::optional<std::string> computeSha256(
        const std::filesystem::path& filePath,
        std::stop_token stopToken,
        std::uint64_t generation) const;
    void attemptDownload(const std::shared_ptr<DownloadSession>& session,
                         std::stop_token stopToken);
    void finishSession(const std::shared_ptr<DownloadSession>& session,
                       model::UpgradeErrorCode errorCode,
                       const std::string& message);
    std::chrono::seconds getRetryDelay(int retryAttempt) const;
    void cancelHttpRequest(const std::string& requestId) const;

    bool enqueueIo(IoJobKind kind,
                   std::chrono::steady_clock::time_point due,
                   WorkerJob job);
    bool enqueueVerify(WorkerJob job);
    void ioWorkerLoop(std::stop_token stopToken);
    void verifyWorkerLoop(std::stop_token stopToken);

    /// Invalidates the active session and schedules cancellation. A soft reset
    /// keeps its path as a resume hint; a hard reset removes the file after
    /// transport cancellation has closed it.
    void retireActiveSession(bool preservePartial, bool deletePartial);

    [[nodiscard]] bool isCurrent(const DownloadSession& session) const
    {
        return session.generation == mGeneration.load(std::memory_order_acquire);
    }

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
    std::shared_ptr<AsyncCallbackGate> mCallbackGate;
    std::filesystem::path mDownloadDir;
    std::atomic<int> mMaxRetryCount{3};
    std::atomic<std::uint64_t> mGeneration{0};
    std::atomic<bool> mStopRequested{false};

    mutable std::mutex mSessionMutex;
    std::shared_ptr<DownloadSession> mActiveSession;
    std::filesystem::path mResumePartialPath;
    std::optional<model::PackageInfo> mResumePackageInfo;
    /// Files created by this manager, including failed downloads whose active
    /// session has already been cleared before the FSM asks for a hard reset.
    std::vector<std::filesystem::path> mTrackedFilePaths;
    /// Includes requests retired from mActiveSession but not yet acknowledged.
    std::unordered_set<std::string> mOutstandingRequestIds;

    std::mutex mIoMutex;
    std::condition_variable_any mIoCV;
    std::deque<IoJob> mIoJobs;
    std::uint64_t mIoRevision{0};
    std::uint64_t mNextIoSequence{0};

    std::mutex mVerifyMutex;
    std::condition_variable_any mVerifyCV;
    std::deque<WorkerJob> mVerifyJobs;

    // Declared last: they are stopped and joined before any state they access
    // is destroyed, including when construction fails partway through.
    std::jthread mIoWorker;
    std::jthread mVerifyWorker;
};

} // namespace ucf::service
