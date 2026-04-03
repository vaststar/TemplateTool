#include "UpgradeDownloadManager.h"
#include "../UpgradeServiceLogger.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/NetworkService/Http/INetworkHttpManager.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToFileRequest.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToFileResponse.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>

// Use OpenSSL for SHA-256 if available; otherwise a stub
#include <openssl/evp.h>

namespace ucf::service {

UpgradeDownloadManager::UpgradeDownloadManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
{
    ensureDownloadDirectory();
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager created, dir=" << mDownloadDir.string());
}

UpgradeDownloadManager::~UpgradeDownloadManager()
{
    cancelDownload();
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager destroyed");
}

void UpgradeDownloadManager::ensureDownloadDirectory()
{
    auto coreFramework = mCoreFramework.lock();
    if (coreFramework) {
        if (auto clientInfo = coreFramework->getService<IClientInfoService>().lock()) {
            mDownloadDir = std::filesystem::path(clientInfo->getAppDataStoragePath()) / "upgrade_downloads";
        }
    }
    if (mDownloadDir.empty()) {
        mDownloadDir = std::filesystem::temp_directory_path() / "template-factory-upgrade";
    }
    std::filesystem::create_directories(mDownloadDir);
}

std::filesystem::path UpgradeDownloadManager::getDownloadDirectory() const
{
    return mDownloadDir;
}

bool UpgradeDownloadManager::hasSufficientSpace(int64_t requiredBytes) const
{
    try {
        auto spaceInfo = std::filesystem::space(mDownloadDir);
        // Need ~3x: download + extraction buffer
        return spaceInfo.available > static_cast<std::uintmax_t>(requiredBytes * 3);
    } catch (const std::exception& ex) {
        UPGRADE_LOG_ERROR("Failed to check disk space: " << ex.what());
        return false;
    }
}

void UpgradeDownloadManager::downloadPackage(
    const model::PackageInfo& packageInfo,
    ProgressCallback progressCb,
    DownloadCompleteCallback completeCb)
{
    if (mDownloading.load()) {
        UPGRADE_LOG_WARN("Download already in progress");
        return;
    }

    // Pre-flight: check disk space
    if (!hasSufficientSpace(packageInfo.sizeBytes)) {
        UPGRADE_LOG_ERROR("Insufficient disk space for download");
        completeCb(false, "", model::UpgradeErrorCode::DiskSpaceError, "Insufficient disk space");
        return;
    }

    mCancelled.store(false);
    mCurrentRetry = 0;
    mLastProgressTime = {};
    attemptDownload(packageInfo, std::move(progressCb), std::move(completeCb));
}

void UpgradeDownloadManager::attemptDownload(
    const model::PackageInfo& packageInfo,
    ProgressCallback progressCb,
    DownloadCompleteCallback completeCb)
{
    auto coreFramework = mCoreFramework.lock();
    if (!coreFramework) {
        completeCb(false, "", model::UpgradeErrorCode::NetworkError, "CoreFramework unavailable");
        return;
    }

    auto networkService = coreFramework->getService<INetworkService>().lock();
    if (!networkService) {
        completeCb(false, "", model::UpgradeErrorCode::NetworkError, "NetworkService unavailable");
        return;
    }

    auto httpManager = networkService->getNetworkHttpManager().lock();
    if (!httpManager) {
        completeCb(false, "", model::UpgradeErrorCode::NetworkError, "HttpManager unavailable");
        return;
    }

    // Determine download file path
    auto fileName = std::filesystem::path(packageInfo.downloadUrl).filename().string();
    if (fileName.empty()) {
        fileName = "upgrade_package.zip";
    }
    mPartialFilePath = mDownloadDir / fileName;

    // Build headers (support resume if partial file exists)
    network::http::NetworkHttpHeaders headers;
    if (mResumedBytes > 0 && std::filesystem::exists(mPartialFilePath)) {
        headers.emplace_back("Range", "bytes=" + std::to_string(mResumedBytes) + "-");
        UPGRADE_LOG_INFO("Resuming download from byte " << mResumedBytes);
    }

    mDownloading.store(true);

    UPGRADE_LOG_INFO("Starting download: " << packageInfo.downloadUrl
                     << " → " << mPartialFilePath.string());

    auto request = std::make_unique<network::http::HttpDownloadToFileRequest>(
        packageInfo.downloadUrl,
        headers,
        300, // 5 min timeout
        mPartialFilePath.string()
    );

    httpManager->downloadContentToFile(*request,
        [this, packageInfo, progressCb, completeCb](
            const network::http::HttpDownloadToFileResponse& response)
        {
            if (mCancelled.load()) {
                mDownloading.store(false);
                return;
            }

            // Progress update
            if (!response.isFinished()) {
                auto current = static_cast<int64_t>(response.getCurrentSize()) + mResumedBytes;
                auto total = static_cast<int64_t>(response.getTotalSize()) + mResumedBytes;

                // Throttle: fire at most every 200ms, or on completion
                bool isComplete = (total > 0 && current >= total);
                auto now = std::chrono::steady_clock::now();
                bool throttleOk = (now - mLastProgressTime >= std::chrono::milliseconds(200));

                if (isComplete || throttleOk) {
                    mLastProgressTime = now;
                    progressCb(current, total);
                }
                return;
            }

            // Download finished
            mDownloading.store(false);

            auto errorData = response.getErrorData();
            if (errorData.has_value()) {
                UPGRADE_LOG_ERROR("Download failed: " << errorData->errorDescription);

                // Retry with exponential backoff
                if (mCurrentRetry < mMaxRetryCount) {
                    mCurrentRetry++;
                    auto delay = getRetryDelay();
                    UPGRADE_LOG_INFO("Retrying download in " << delay.count()
                                     << "s (attempt " << mCurrentRetry << "/" << mMaxRetryCount << ")");
                    // Record partial progress for resume
                    if (std::filesystem::exists(mPartialFilePath)) {
                        mResumedBytes = static_cast<int64_t>(std::filesystem::file_size(mPartialFilePath));
                    }
                    // Retry after delay (on a detached thread — simple approach)
                    std::thread([this, packageInfo, progressCb, completeCb, delay]() {
                        std::this_thread::sleep_for(delay);
                        if (!mCancelled.load()) {
                            attemptDownload(packageInfo, progressCb, completeCb);
                        }
                    }).detach();
                    return;
                }

                completeCb(false, "", model::UpgradeErrorCode::DownloadFailed,
                           "Download failed after " + std::to_string(mMaxRetryCount) + " retries: "
                           + errorData->errorDescription);
                return;
            }

            UPGRADE_LOG_INFO("Download complete: " << mPartialFilePath.string());
            mResumedBytes = 0;
            completeCb(true, mPartialFilePath.string(), model::UpgradeErrorCode::None, "");
        });
}

void UpgradeDownloadManager::cancelDownload()
{
    mCancelled.store(true);
    mDownloading.store(false);
    UPGRADE_LOG_INFO("Download cancelled");
}

void UpgradeDownloadManager::verifyPackage(
    const std::string& filePath,
    const std::string& expectedSha256,
    VerifyCompleteCallback callback)
{
    UPGRADE_LOG_INFO("Verifying package: " << filePath);

    // Run verification on a background thread to avoid blocking FSM
    std::thread([this, filePath, expectedSha256, callback]() {
        try {
            if (!std::filesystem::exists(filePath)) {
                callback(false, model::UpgradeErrorCode::VerifyFailed, "File not found: " + filePath);
                return;
            }

            auto actualHash = computeSha256(std::filesystem::path(filePath));

            if (expectedSha256.empty()) {
                UPGRADE_LOG_WARN("No expected SHA-256 provided, skipping verification");
                callback(true, model::UpgradeErrorCode::None, "");
                return;
            }

            if (actualHash != expectedSha256) {
                UPGRADE_LOG_ERROR("SHA-256 mismatch: expected=" << expectedSha256
                                  << " actual=" << actualHash);
                callback(false, model::UpgradeErrorCode::VerifyFailed,
                         "SHA-256 mismatch: expected " + expectedSha256 + ", got " + actualHash);
                return;
            }

            UPGRADE_LOG_INFO("Package verification passed");
            callback(true, model::UpgradeErrorCode::None, "");
        } catch (const std::exception& ex) {
            UPGRADE_LOG_ERROR("Verification exception: " << ex.what());
            callback(false, model::UpgradeErrorCode::VerifyFailed, ex.what());
        }
    }).detach();
}

std::string UpgradeDownloadManager::computeSha256(const std::filesystem::path& filePath) const
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for SHA-256: " + filePath.string());
    }

    auto ctx = EVP_MD_CTX_new();
    if (!ctx) {
        throw std::runtime_error("EVP_MD_CTX_new failed");
    }

    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);

    char buffer[8192];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        EVP_DigestUpdate(ctx, buffer, static_cast<size_t>(file.gcount()));
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;
    EVP_DigestFinal_ex(ctx, hash, &hashLen);
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < hashLen; ++i) {
        oss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(hash[i]);
    }
    return oss.str();
}

void UpgradeDownloadManager::softReset()
{
    cancelDownload();
    mCurrentRetry = 0;
    mLastProgressTime = {};
    // Partial file and mResumedBytes intentionally preserved for resume
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager soft reset");
}

void UpgradeDownloadManager::hardReset()
{
    softReset();
    mResumedBytes = 0;
    if (!mPartialFilePath.empty() && std::filesystem::exists(mPartialFilePath)) {
        std::filesystem::remove(mPartialFilePath);
        UPGRADE_LOG_DEBUG("Removed partial file: " << mPartialFilePath.string());
    }
    mPartialFilePath.clear();
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager hard reset");
}

void UpgradeDownloadManager::setMaxRetryCount(int count)
{
    mMaxRetryCount = count;
}

std::chrono::seconds UpgradeDownloadManager::getRetryDelay() const
{
    // Exponential backoff: 2s, 4s, 8s ...
    return std::chrono::seconds(1 << (mCurrentRetry));
}

} // namespace ucf::service
