#pragma once

#include <ucf/Services/UpgradeService/UpgradeModel.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

/// Responsible for downloading upgrade packages and verifying their integrity.
class UpgradeDownloadManager final
{
public:
    explicit UpgradeDownloadManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~UpgradeDownloadManager();

    UpgradeDownloadManager(const UpgradeDownloadManager&) = delete;
    UpgradeDownloadManager& operator=(const UpgradeDownloadManager&) = delete;

    /// Progress callback (throttled: ~200ms intervals, 100% always fires)
    using ProgressCallback = std::function<void(int64_t currentBytes, int64_t totalBytes)>;

    /// Download completion callback
    using DownloadCompleteCallback = std::function<void(
        bool success,
        const std::string& filePath,
        model::UpgradeErrorCode errorCode,
        const std::string& errorMessage)>;

    /// Verify completion callback
    using VerifyCompleteCallback = std::function<void(
        bool success,
        model::UpgradeErrorCode errorCode,
        const std::string& errorMessage)>;

    /// Start downloading a package (async). Any prior in-flight download is
    /// invalidated via generation bump; its lingering callbacks become no-ops.
    void downloadPackage(const model::PackageInfo& packageInfo,
                         ProgressCallback progressCb,
                         DownloadCompleteCallback completeCb);

    /// Cancel an in-progress download. Bumps the generation so any future
    /// callbacks from the underlying HTTP layer are ignored.
    void cancelDownload();

    /// Verify the downloaded file against expected SHA-256
    void verifyPackage(const std::string& filePath,
                       const std::string& expectedSha256,
                       VerifyCompleteCallback callback);

    /// Get the download directory path
    [[nodiscard]] std::filesystem::path getDownloadDirectory() const;

    /// Check whether a download is in progress
    [[nodiscard]] bool isDownloading() const;

    /// Check whether there is sufficient disk space
    [[nodiscard]] bool hasSufficientSpace(int64_t requiredBytes) const;

    /// Soft reset: cancel download, keep partial file (for resume)
    void softReset();

    /// Hard reset: soft reset + delete partial file
    void hardReset();

    /// Set maximum retry count (default: 3)
    void setMaxRetryCount(int count);

private:
    /// Per-download state. One instance per downloadPackage() call.
    /// Captured by shared_ptr into HTTP callbacks so each download has its
    /// own isolated state and lingering callbacks from prior downloads can
    /// be filtered out via the generation token.
    struct DownloadSession
    {
        std::uint64_t generation{0};
        model::PackageInfo packageInfo{};
        ProgressCallback progressCb;
        DownloadCompleteCallback completeCb;
        std::filesystem::path partialFilePath;
        std::int64_t resumedBytes{0};
        int currentRetry{0};
        std::chrono::steady_clock::time_point lastProgressTime{};
    };

    void ensureDownloadDirectory();
    std::string computeSha256(const std::filesystem::path& filePath) const;
    void attemptDownload(std::shared_ptr<DownloadSession> session);
    std::chrono::seconds getRetryDelay(int retryAttempt) const;

    /// True if `session` is still the active generation.
    bool isCurrent(const DownloadSession& session) const
    {
        return session.generation == mGeneration.load(std::memory_order_acquire);
    }

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
    std::filesystem::path mDownloadDir;
    int mMaxRetryCount{3};

    /// Monotonically increasing token. Bumped on every new download or cancel.
    std::atomic<std::uint64_t> mGeneration{0};

    mutable std::mutex mSessionMutex;
    /// Currently active session (null when idle). Guarded by mSessionMutex.
    std::shared_ptr<DownloadSession> mActiveSession;

    /// Resume hint preserved across soft resets so the next downloadPackage()
    /// can pick up where the previous attempt left off. Cleared by hardReset.
    std::filesystem::path mResumePartialPath;
    std::int64_t mResumeBytes{0};
};

} // namespace ucf::service
