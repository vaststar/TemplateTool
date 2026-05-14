#include "UpgradeDownloadManager.h"
#include "../UpgradeConstants.h"
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
#include <utility>

// Use OpenSSL for SHA-256 if available; otherwise a stub
#include <openssl/evp.h>

namespace ucf::service {

UpgradeDownloadManager::UpgradeDownloadManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
{
    ensureDownloadDirectory();
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager created, dir=" << ucf::utilities::FilePathUtils::utf8FromPath(mDownloadDir));
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
            mDownloadDir = ucf::utilities::FilePathUtils::pathFromUtf8(clientInfo->getAppCacheStoragePath()) / upgrade::constants::kDownloadSubDir;
        }
    }
    if (mDownloadDir.empty()) {
        mDownloadDir = std::filesystem::temp_directory_path() / upgrade::constants::kTempFallbackAppName / upgrade::constants::kDownloadSubDir;
    }
    ucf::utilities::FilePathUtils::createDirectoriesUtf8(ucf::utilities::FilePathUtils::utf8FromPath(mDownloadDir));
}

std::filesystem::path UpgradeDownloadManager::getDownloadDirectory() const
{
    return mDownloadDir;
}

bool UpgradeDownloadManager::isDownloading() const
{
    std::lock_guard<std::mutex> lk(mSessionMutex);
    return static_cast<bool>(mActiveSession);
}

bool UpgradeDownloadManager::hasSufficientSpace(int64_t requiredBytes) const
{
    try {
        ucf::utilities::FilePathUtils::createDirectoriesUtf8(ucf::utilities::FilePathUtils::utf8FromPath(mDownloadDir));
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
    // Pre-flight: check disk space
    if (!hasSufficientSpace(packageInfo.sizeBytes)) {
        UPGRADE_LOG_ERROR("Insufficient disk space for download");
        completeCb(false, "", model::UpgradeErrorCode::DiskSpaceError, "Insufficient disk space");
        return;
    }

    // Carry over partial bytes from any prior session (resume support).
    std::int64_t resumeBytes = 0;
    std::filesystem::path partialPath;
    {
        std::lock_guard<std::mutex> lk(mSessionMutex);
        resumeBytes = mResumeBytes;
        partialPath = mResumePartialPath;
    }

    // Bump generation: any in-flight callbacks from the previous session
    // will see a stale generation and become no-ops.
    auto newGeneration = mGeneration.fetch_add(1, std::memory_order_acq_rel) + 1;

    auto session = std::make_shared<DownloadSession>();
    session->generation = newGeneration;
    session->packageInfo = packageInfo;
    session->progressCb = std::move(progressCb);
    session->completeCb = std::move(completeCb);
    session->resumedBytes = resumeBytes;
    session->partialFilePath = std::move(partialPath);

    {
        std::lock_guard<std::mutex> lk(mSessionMutex);
        mActiveSession = session;
        // Clear resume hint — it's now owned by the active session.
        mResumePartialPath.clear();
        mResumeBytes = 0;
    }

    attemptDownload(std::move(session));
}

void UpgradeDownloadManager::attemptDownload(std::shared_ptr<DownloadSession> session)
{
    if (!isCurrent(*session)) {
        return;
    }

    auto coreFramework = mCoreFramework.lock();
    if (!coreFramework) {
        session->completeCb(false, "", model::UpgradeErrorCode::NetworkError, "CoreFramework unavailable");
        return;
    }

    auto networkService = coreFramework->getService<INetworkService>().lock();
    if (!networkService) {
        session->completeCb(false, "", model::UpgradeErrorCode::NetworkError, "NetworkService unavailable");
        return;
    }

    auto httpManager = networkService->getNetworkHttpManager().lock();
    if (!httpManager) {
        session->completeCb(false, "", model::UpgradeErrorCode::NetworkError, "HttpManager unavailable");
        return;
    }

    // Determine download file path (sticky across retries within a session).
    if (session->partialFilePath.empty()) {
        auto fileName = ucf::utilities::FilePathUtils::pathFromUtf8(session->packageInfo.downloadUrl).filename().string();
        if (fileName.empty()) {
            fileName = upgrade::constants::kDefaultPackageFileName;
        }
        session->partialFilePath = mDownloadDir / fileName;
    }

    // Build headers (support resume if partial file exists)
    network::http::NetworkHttpHeaders headers;
    if (session->resumedBytes > 0 && std::filesystem::exists(session->partialFilePath)) {
        headers.emplace_back("Range", "bytes=" + std::to_string(session->resumedBytes) + "-");
        UPGRADE_LOG_INFO("Resuming download from byte " << session->resumedBytes);
    }

    UPGRADE_LOG_INFO("Starting download (gen=" << session->generation << "): "
                     << session->packageInfo.downloadUrl
                     << " → " << ucf::utilities::FilePathUtils::utf8FromPath(session->partialFilePath));

    auto request = std::make_unique<network::http::HttpDownloadToFileRequest>(
        session->packageInfo.downloadUrl,
        headers,
        300, // 5 min timeout
        ucf::utilities::FilePathUtils::utf8FromPath(session->partialFilePath)
    );

    // Capture requestId so cancelDownload() can abort the transport-level
    // transfer. The HTTP layer will deliver a terminal callback with
    // errorType=CanceledError, which the isCurrent() gate will discard.
    session->requestId = request->getRequestId();

    httpManager->downloadContentToFile(*request,
        [this, session](const network::http::HttpDownloadToFileResponse& response)
        {
            // First gate: stale callback from a cancelled / superseded session.
            if (!isCurrent(*session)) {
                return;
            }

            // Progress update
            if (!response.isFinished()) {
                auto current = static_cast<int64_t>(response.getCurrentSize()) + session->resumedBytes;
                auto total = static_cast<int64_t>(response.getTotalSize()) + session->resumedBytes;

                // Throttle: fire at most every 200ms, or on completion
                bool isComplete = (total > 0 && current >= total);
                auto now = std::chrono::steady_clock::now();
                bool throttleOk = (now - session->lastProgressTime >= std::chrono::milliseconds(200));

                if (isComplete || throttleOk) {
                    session->lastProgressTime = now;
                    session->progressCb(current, total);
                }
                return;
            }

            // Download finished — re-check generation under lock and clear
            // active slot atomically with the completion notification.
            auto errorData = response.getErrorData();
            if (errorData.has_value()) {
                UPGRADE_LOG_ERROR("Download failed (gen=" << session->generation
                                  << "): " << errorData->errorDescription);

                // Retry with exponential backoff
                if (session->currentRetry < mMaxRetryCount) {
                    session->currentRetry++;
                    auto delay = getRetryDelay(session->currentRetry);
                    UPGRADE_LOG_INFO("Retrying download in " << delay.count()
                                     << "s (attempt " << session->currentRetry << "/" << mMaxRetryCount << ")");
                    // Record partial progress for resume
                    if (std::filesystem::exists(session->partialFilePath)) {
                        session->resumedBytes = static_cast<int64_t>(std::filesystem::file_size(session->partialFilePath));
                    }
                    // Retry after delay (on a detached thread)
                    std::thread([this, session, delay]() {
                        std::this_thread::sleep_for(delay);
                        if (isCurrent(*session)) {
                            attemptDownload(session);
                        }
                    }).detach();
                    return;
                }

                {
                    std::lock_guard<std::mutex> lk(mSessionMutex);
                    if (mActiveSession.get() == session.get()) {
                        mActiveSession.reset();
                    }
                }
                session->completeCb(false, "", model::UpgradeErrorCode::DownloadFailed,
                           "Download failed after " + std::to_string(mMaxRetryCount) + " retries: "
                           + errorData->errorDescription);
                return;
            }

            UPGRADE_LOG_INFO("Download complete: " << ucf::utilities::FilePathUtils::utf8FromPath(session->partialFilePath));
            auto finalPath = ucf::utilities::FilePathUtils::utf8FromPath(session->partialFilePath);
            {
                std::lock_guard<std::mutex> lk(mSessionMutex);
                if (mActiveSession.get() == session.get()) {
                    mActiveSession.reset();
                }
            }
            session->completeCb(true, finalPath, model::UpgradeErrorCode::None, "");
        });
}

void UpgradeDownloadManager::cancelDownload()
{
    // Bump generation first so any in-flight HTTP callbacks become no-ops.
    mGeneration.fetch_add(1, std::memory_order_acq_rel);

    std::shared_ptr<DownloadSession> dropped;
    {
        std::lock_guard<std::mutex> lk(mSessionMutex);
        dropped = std::move(mActiveSession);
    }
    if (!dropped) {
        return;
    }

    UPGRADE_LOG_INFO("Download cancelled (gen=" << dropped->generation << ")");

    // Abort the underlying HTTP transfer so we don't keep downloading bytes
    // for a session that no caller will observe. The terminal callback that
    // results (errorType=CanceledError) is automatically discarded by the
    // isCurrent() gate at the top of the response lambda.
    if (dropped->requestId.empty()) {
        return;
    }
    auto coreFramework = mCoreFramework.lock();
    if (!coreFramework) {
        return;
    }
    auto networkService = coreFramework->getService<INetworkService>().lock();
    if (!networkService) {
        return;
    }
    auto httpManager = networkService->getNetworkHttpManager().lock();
    if (!httpManager) {
        return;
    }
    httpManager->cancelRequest(dropped->requestId);
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
            if (!ucf::utilities::FilePathUtils::existsUtf8(filePath)) {
                callback(false, model::UpgradeErrorCode::VerifyFailed, "File not found: " + filePath);
                return;
            }

            auto actualHash = computeSha256(ucf::utilities::FilePathUtils::pathFromUtf8(filePath));

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
    // Preserve partial-file info as a resume hint for the next download.
    std::shared_ptr<DownloadSession> previous;
    {
        std::lock_guard<std::mutex> lk(mSessionMutex);
        previous = mActiveSession;
    }
    cancelDownload();
    if (previous && !previous->partialFilePath.empty()) {
        std::lock_guard<std::mutex> lk(mSessionMutex);
        mResumePartialPath = previous->partialFilePath;
        if (std::filesystem::exists(mResumePartialPath)) {
            mResumeBytes = static_cast<int64_t>(std::filesystem::file_size(mResumePartialPath));
        } else {
            mResumeBytes = 0;
        }
    }
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager soft reset");
}

void UpgradeDownloadManager::hardReset()
{
    std::filesystem::path partialToDelete;
    {
        std::lock_guard<std::mutex> lk(mSessionMutex);
        if (mActiveSession) {
            partialToDelete = mActiveSession->partialFilePath;
        } else if (!mResumePartialPath.empty()) {
            partialToDelete = mResumePartialPath;
        }
    }
    cancelDownload();
    {
        std::lock_guard<std::mutex> lk(mSessionMutex);
        mResumePartialPath.clear();
        mResumeBytes = 0;
    }
    if (!partialToDelete.empty() && std::filesystem::exists(partialToDelete)) {
        std::error_code ec;
        std::filesystem::remove(partialToDelete, ec);
        UPGRADE_LOG_DEBUG("Removed partial file: " << ucf::utilities::FilePathUtils::utf8FromPath(partialToDelete));
    }
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager hard reset");
}

void UpgradeDownloadManager::setMaxRetryCount(int count)
{
    mMaxRetryCount = count;
}

std::chrono::seconds UpgradeDownloadManager::getRetryDelay(int retryAttempt) const
{
    // Exponential backoff: 2s, 4s, 8s ...
    return std::chrono::seconds(1 << retryAttempt);
}

} // namespace ucf::service
