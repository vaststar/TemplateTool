#pragma once

#include <ucf/Services/UpgradeService/UpgradeModel.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
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

    /// Start downloading a package (async)
    void downloadPackage(const model::PackageInfo& packageInfo,
                         ProgressCallback progressCb,
                         DownloadCompleteCallback completeCb);

    /// Cancel an in-progress download
    void cancelDownload();

    /// Verify the downloaded file against expected SHA-256
    void verifyPackage(const std::string& filePath,
                       const std::string& expectedSha256,
                       VerifyCompleteCallback callback);

    /// Get the download directory path
    [[nodiscard]] std::filesystem::path getDownloadDirectory() const;

    /// Check whether a download is in progress
    [[nodiscard]] bool isDownloading() const { return mDownloading.load(); }

    /// Check whether there is sufficient disk space
    [[nodiscard]] bool hasSufficientSpace(int64_t requiredBytes) const;

    /// Soft reset: cancel download, reset retry state, keep partial file (for resume)
    void softReset();

    /// Hard reset: soft reset + delete partial file
    void hardReset();

    /// Set maximum retry count (default: 3)
    void setMaxRetryCount(int count);

private:
    void ensureDownloadDirectory();
    std::string computeSha256(const std::filesystem::path& filePath) const;
    void attemptDownload(const model::PackageInfo& packageInfo,
                         ProgressCallback progressCb,
                         DownloadCompleteCallback completeCb);
    std::chrono::seconds getRetryDelay() const;

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
    std::atomic<bool> mDownloading{false};
    std::atomic<bool> mCancelled{false};
    std::filesystem::path mDownloadDir;
    std::filesystem::path mPartialFilePath;
    int64_t mResumedBytes{0};

    int mMaxRetryCount{3};
    int mCurrentRetry{0};

    std::chrono::steady_clock::time_point mLastProgressTime{};
};

} // namespace ucf::service
