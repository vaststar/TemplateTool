#include "UpgradeDownloadManager.h"
#include "../AsyncCallbackGate.h"
#include "../UpgradeConstants.h"
#include "../UpgradeServiceLogger.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/NetworkService/INetworkService.h>
#include <ucf/services/NetworkService/http/INetworkHttpManager.h>
#include <ucf/services/NetworkService/model/HttpDownloadToFileRequest.h>
#include <ucf/services/NetworkService/model/HttpDownloadToFileResponse.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>
#include <ucf/utilities/FilePathUtils/FilePathUtils.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <openssl/evp.h>

namespace ucf::service {

UpgradeDownloadManager::UpgradeDownloadManager(
    ucf::framework::ICoreFrameworkWPtr coreFramework,
    std::shared_ptr<AsyncCallbackGate> callbackGate)
    : mCoreFramework(std::move(coreFramework))
    , mCallbackGate(std::move(callbackGate))
    , mIoWorker([this](std::stop_token st) { ioWorkerLoop(st); })
    , mVerifyWorker([this](std::stop_token st) { verifyWorkerLoop(st); })
{
    ensureDownloadDirectory();
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager constructed, address: " << this
                      << ", directory: " << ucf::utilities::FilePathUtils::utf8FromPath(mDownloadDir));
}

UpgradeDownloadManager::~UpgradeDownloadManager()
{
    // The owning UpgradeManager normally closes and drains the shared gate
    // before destroying us. Keep this fallback for standalone destruction.
    if (mCallbackGate) {
        mCallbackGate->close();
    }
    requestStop();
    cancelOutstandingRequests();
    joinWorkers();
    if (mCallbackGate) {
        mCallbackGate->waitForDrain();
    }
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager destroying, address: " << this);
}

void UpgradeDownloadManager::ensureDownloadDirectory()
{
    if (auto coreFramework = mCoreFramework.lock()) {
        if (auto clientInfo = coreFramework->getService<IClientInfoService>().lock()) {
            mDownloadDir = ucf::utilities::FilePathUtils::pathFromUtf8(
                clientInfo->getAppCacheStoragePath()) / upgrade::constants::kDownloadSubDir;
        }
    }
    if (mDownloadDir.empty()) {
        mDownloadDir = std::filesystem::temp_directory_path()
                     / upgrade::constants::kTempFallbackAppName
                     / upgrade::constants::kDownloadSubDir;
    }
    ucf::utilities::FilePathUtils::createDirectoriesUtf8(
        ucf::utilities::FilePathUtils::utf8FromPath(mDownloadDir));
}

std::filesystem::path UpgradeDownloadManager::getDownloadDirectory() const
{
    return mDownloadDir;
}

bool UpgradeDownloadManager::isDownloading() const
{
    std::lock_guard lock(mSessionMutex);
    return !mStopRequested.load(std::memory_order_acquire)
        && static_cast<bool>(mActiveSession);
}

bool UpgradeDownloadManager::hasSufficientSpace(int64_t requiredBytes) const
{
    try {
        ucf::utilities::FilePathUtils::createDirectoriesUtf8(
            ucf::utilities::FilePathUtils::utf8FromPath(mDownloadDir));
        auto spaceInfo = std::filesystem::space(mDownloadDir);
        return spaceInfo.available > static_cast<std::uintmax_t>(requiredBytes * 3);
    } catch (const std::exception& ex) {
        UPGRADE_LOG_ERROR("Failed to check disk space: " << ex.what());
        return false;
    }
}

bool UpgradeDownloadManager::enqueueIo(
    IoJobKind kind,
    std::chrono::steady_clock::time_point due,
    WorkerJob job)
{
    {
        std::lock_guard lock(mIoMutex);
        // A callback already admitted by the gate can request a hard reset
        // after shutdown has stopped the worker. Keep its cleanup task for
        // joinWorkers() to run after transport cancellation.
        if (mStopRequested.load(std::memory_order_acquire)
            && kind != IoJobKind::Cleanup) {
            return false;
        }
        mIoJobs.push_back(IoJob{kind, due, mNextIoSequence++, std::move(job)});
        ++mIoRevision;
    }
    mIoCV.notify_one();
    return true;
}

bool UpgradeDownloadManager::enqueueVerify(WorkerJob job)
{
    {
        std::lock_guard lock(mVerifyMutex);
        if (mStopRequested.load(std::memory_order_acquire)) {
            return false;
        }
        mVerifyJobs.push_back(std::move(job));
    }
    mVerifyCV.notify_one();
    return true;
}

void UpgradeDownloadManager::ioWorkerLoop(std::stop_token stopToken)
{
    while (!stopToken.stop_requested()) {
        IoJob job{};
        {
            std::unique_lock lock(mIoMutex);
            for (;;) {
                if (stopToken.stop_requested()) {
                    return;
                }
                if (mIoJobs.empty()) {
                    mIoCV.wait(lock, stopToken, [this] { return !mIoJobs.empty(); });
                    continue;
                }

                const auto now = std::chrono::steady_clock::now();
                auto ready = mIoJobs.end();
                auto earliest = mIoJobs.front().due;
                for (auto it = mIoJobs.begin(); it != mIoJobs.end(); ++it) {
                    earliest = std::min(earliest, it->due);
                    if (it->due > now) {
                        continue;
                    }
                    if (ready == mIoJobs.end()
                        || it->kind < ready->kind
                        || (it->kind == ready->kind && it->sequence < ready->sequence)) {
                        ready = it;
                    }
                }
                if (ready != mIoJobs.end()) {
                    job = std::move(*ready);
                    mIoJobs.erase(ready);
                    break;
                }

                const auto revision = mIoRevision;
                mIoCV.wait_until(lock, stopToken, earliest,
                    [this, revision] { return mIoRevision != revision; });
            }
        }

        try {
            job.run(stopToken);
        } catch (const std::exception& ex) {
            UPGRADE_LOG_ERROR("Upgrade I/O job failed: " << ex.what());
        } catch (...) {
            UPGRADE_LOG_ERROR("Upgrade I/O job failed with an unknown exception");
        }
    }
}

void UpgradeDownloadManager::verifyWorkerLoop(std::stop_token stopToken)
{
    while (!stopToken.stop_requested()) {
        WorkerJob job;
        {
            std::unique_lock lock(mVerifyMutex);
            mVerifyCV.wait(lock, stopToken, [this] { return !mVerifyJobs.empty(); });
            if (stopToken.stop_requested()) {
                return;
            }
            job = std::move(mVerifyJobs.front());
            mVerifyJobs.pop_front();
        }

        try {
            job(stopToken);
        } catch (const std::exception& ex) {
            UPGRADE_LOG_ERROR("Upgrade verification job failed: " << ex.what());
        } catch (...) {
            UPGRADE_LOG_ERROR("Upgrade verification job failed with an unknown exception");
        }
    }
}

void UpgradeDownloadManager::downloadPackage(
    const model::PackageInfo& packageInfo,
    ProgressCallback progressCb,
    DownloadCompleteCallback completeCb)
{
    if (mStopRequested.load(std::memory_order_acquire)) {
        return;
    }
    if (!hasSufficientSpace(packageInfo.sizeBytes)) {
        UPGRADE_LOG_ERROR("Insufficient disk space for download");
        if (!mStopRequested.load(std::memory_order_acquire)) {
            completeCb(false, "", model::UpgradeErrorCode::DiskSpaceError,
                       "Insufficient disk space");
        }
        return;
    }

    auto session = std::make_shared<DownloadSession>();
    session->packageInfo = packageInfo;
    session->progressCb = std::move(progressCb);
    session->completeCb = std::move(completeCb);

    std::string previousRequestId;
    std::filesystem::path staleResumePath;
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire)) {
            return;
        }
        session->generation = mGeneration.fetch_add(1, std::memory_order_acq_rel) + 1;
        if (mActiveSession) {
            previousRequestId = mActiveSession->requestId;
        }
        const bool samePackage = mResumePackageInfo.has_value()
            && !packageInfo.sha256.empty()
            && mResumePackageInfo->downloadUrl == packageInfo.downloadUrl
            && mResumePackageInfo->sha256 == packageInfo.sha256
            && mResumePackageInfo->sizeBytes == packageInfo.sizeBytes;
        if (samePackage) {
            session->partialFilePath = std::move(mResumePartialPath);
            session->resumeAllowed = !session->partialFilePath.empty();
        } else {
            staleResumePath = std::move(mResumePartialPath);
        }
        mResumePartialPath.clear();
        mResumePackageInfo.reset();
        mActiveSession = session;
    }

    const auto now = std::chrono::steady_clock::now();
    if (!previousRequestId.empty()) {
        enqueueIo(IoJobKind::Cancel, now,
            [this, previousRequestId](std::stop_token) {
                cancelHttpRequest(previousRequestId);
            });
    }
    if (!staleResumePath.empty()) {
        enqueueIo(IoJobKind::Cleanup, now,
            [staleResumePath](std::stop_token) {
                std::error_code ec;
                std::filesystem::remove(staleResumePath, ec);
                if (ec) {
                    UPGRADE_LOG_WARN("Failed to remove stale partial download: " << ec.message());
                }
            });
    }
    enqueueIo(IoJobKind::Attempt, now,
        [this, session](std::stop_token st) { attemptDownload(session, st); });
}

void UpgradeDownloadManager::finishSession(
    const std::shared_ptr<DownloadSession>& session,
    model::UpgradeErrorCode errorCode,
    const std::string& message)
{
    DownloadCompleteCallback callback;
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire)
            || !isCurrent(*session)
            || mActiveSession.get() != session.get()) {
            return;
        }
        mActiveSession.reset();
        callback = session->completeCb;
    }
    if (callback) {
        callback(false, "", errorCode, message);
    }
}

void UpgradeDownloadManager::attemptDownload(
    const std::shared_ptr<DownloadSession>& session,
    std::stop_token stopToken)
{
    if (stopToken.stop_requested()
        || mStopRequested.load(std::memory_order_acquire)
        || !isCurrent(*session)) {
        return;
    }

    auto coreFramework = mCoreFramework.lock();
    if (!coreFramework) {
        finishSession(session, model::UpgradeErrorCode::NetworkError,
                      "CoreFramework unavailable");
        return;
    }
    auto networkService = coreFramework->getService<INetworkService>().lock();
    if (!networkService) {
        finishSession(session, model::UpgradeErrorCode::NetworkError,
                      "NetworkService unavailable");
        return;
    }
    auto httpManager = networkService->getNetworkHttpManager().lock();
    if (!httpManager) {
        finishSession(session, model::UpgradeErrorCode::NetworkError,
                      "HttpManager unavailable");
        return;
    }

    std::filesystem::path partialPath;
    bool resumeAllowed = false;
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire)
            || !isCurrent(*session)
            || mActiveSession.get() != session.get()) {
            return;
        }
        if (session->partialFilePath.empty()) {
            auto fileName = ucf::utilities::FilePathUtils::pathFromUtf8(
                session->packageInfo.downloadUrl).filename().string();
            if (fileName.empty()) {
                fileName = upgrade::constants::kDefaultPackageFileName;
            }
            session->partialFilePath = mDownloadDir / fileName;
        }
        if (std::find(mTrackedFilePaths.begin(), mTrackedFilePaths.end(),
                      session->partialFilePath) == mTrackedFilePaths.end()) {
            mTrackedFilePaths.push_back(session->partialFilePath);
        }
        partialPath = session->partialFilePath;
        resumeAllowed = session->resumeAllowed;
    }

    std::int64_t resumedBytes = 0;
    if (resumeAllowed) {
        std::error_code ec;
        if (std::filesystem::exists(partialPath, ec) && !ec) {
            auto size = std::filesystem::file_size(partialPath, ec);
            if (!ec) {
                resumedBytes = static_cast<std::int64_t>(size);
            }
        }
    }

    network::http::NetworkHttpHeaders headers;
    if (resumedBytes > 0) {
        headers.emplace_back("Range", "bytes=" + std::to_string(resumedBytes) + "-");
        UPGRADE_LOG_INFO("Resuming download from byte " << resumedBytes);
    }

    UPGRADE_LOG_INFO("Starting download (gen=" << session->generation << "): "
                     << session->packageInfo.downloadUrl << " → "
                     << ucf::utilities::FilePathUtils::utf8FromPath(partialPath));

    auto request = std::make_unique<network::http::HttpDownloadToFileRequest>(
        session->packageInfo.downloadUrl,
        headers,
        300,
        ucf::utilities::FilePathUtils::utf8FromPath(partialPath));
    const std::string requestId = request->getRequestId();

    // Publish the ID before sending. If shutdown cancels it before the HTTP
    // layer enqueues it, the post-send check below cancels it again.
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire)
            || !isCurrent(*session)
            || mActiveSession.get() != session.get()) {
            return;
        }
        session->resumedBytes = resumedBytes;
        session->requestId = requestId;
        mOutstandingRequestIds.insert(requestId);
    }

    auto gate = mCallbackGate;
    try {
        httpManager->downloadContentToFile(*request,
            [this, gate, session, requestId](const network::http::HttpDownloadToFileResponse& response) {
                auto lease = gate->tryEnter();
                if (!lease) {
                    return;
                }

                if (!response.isFinished()) {
                    ProgressCallback progress;
                    std::int64_t current = 0;
                    std::int64_t total = 0;
                    {
                        std::lock_guard lock(mSessionMutex);
                        if (mStopRequested.load(std::memory_order_acquire)
                            || !isCurrent(*session)
                            || mActiveSession.get() != session.get()
                            || session->requestId != requestId) {
                            return;
                        }
                        // Redirect/error response bodies are not package
                        // progress; the network layer may follow redirects.
                        const int statusCode = response.getHttpResponseCode();
                        if (statusCode != 200 && statusCode != 206) {
                            return;
                        }
                        // A server may ignore Range and send the whole file.
                        // The HTTP handler truncates in that case, so progress
                        // must start at zero as well.
                        if (statusCode == 200) {
                            session->resumedBytes = 0;
                        }
                        current = static_cast<std::int64_t>(response.getCurrentSize())
                                + session->resumedBytes;
                        total = static_cast<std::int64_t>(response.getTotalSize())
                              + session->resumedBytes;
                        const auto now = std::chrono::steady_clock::now();
                        const bool complete = total > 0 && current >= total;
                        if (complete || now - session->lastProgressTime >= std::chrono::milliseconds(200)) {
                            session->lastProgressTime = now;
                            progress = session->progressCb;
                        }
                    }
                    if (progress) {
                        progress(current, total);
                    }
                    return;
                }

                DownloadCompleteCallback completion;
                std::string finalPath;
                std::string errorMessage;
                bool success = false;
                int retryAttempt = 0;
                std::filesystem::path retryCleanupPath;
                auto errorData = response.getErrorData();
                const int statusCode = response.getHttpResponseCode();
                const bool httpError = statusCode < 200 || statusCode >= 300;
                const bool transportFailed = errorData.has_value() || httpError;
                std::string failureDescription = errorData.has_value()
                    ? errorData->errorDescription
                    : "HTTP " + std::to_string(statusCode);

                // The file handler has closed its stream before the terminal
                // callback. Check the saved file, including an empty 206 body
                // for which the handler never had to open the file.
                bool invalidFileSize = false;
                if (!transportFailed) {
                    if (response.getCurrentSize() == 0) {
                        invalidFileSize = true;
                        failureDescription = "Download response has no package bytes";
                    }
                    std::filesystem::path downloadedPath;
                    {
                        std::lock_guard lock(mSessionMutex);
                        downloadedPath = session->partialFilePath;
                    }
                    std::error_code ec;
                    const auto actualSize = std::filesystem::file_size(downloadedPath, ec);
                    if (ec) {
                        invalidFileSize = true;
                        failureDescription = "Downloaded file is unavailable: " + ec.message();
                    } else if (session->packageInfo.sizeBytes > 0
                               && actualSize != static_cast<std::uintmax_t>(session->packageInfo.sizeBytes)) {
                        invalidFileSize = true;
                        failureDescription = "Downloaded size " + std::to_string(actualSize)
                            + " differs from expected "
                            + std::to_string(session->packageInfo.sizeBytes);
                    } else if (session->packageInfo.sizeBytes <= 0 && actualSize == 0) {
                        invalidFileSize = true;
                        failureDescription = "Downloaded file is empty";
                    }
                }
                const bool failed = transportFailed || invalidFileSize;
                {
                    std::lock_guard lock(mSessionMutex);
                    // Consume a terminal response once. A duplicate completion
                    // must not schedule another retry or complete twice.
                    if (mOutstandingRequestIds.erase(requestId) == 0
                        || session->requestId != requestId) {
                        return;
                    }
                    session->requestId.clear();
                    if (mStopRequested.load(std::memory_order_acquire)
                        || !isCurrent(*session)
                        || mActiveSession.get() != session.get()) {
                        return;
                    }

                    if (failed
                        && session->currentRetry < mMaxRetryCount.load(std::memory_order_acquire)) {
                        retryAttempt = ++session->currentRetry;
                        // An HTTP error body is not part of the package and
                        // must not become the prefix of the next Range request.
                        const bool restartFromZero = (httpError && statusCode >= 300)
                            || invalidFileSize
                            || session->packageInfo.sha256.empty()
                            || (errorData.has_value()
                                && errorData->errorType == network::http::ResponseErrorType::OtherError);
                        if (restartFromZero) {
                            retryCleanupPath = session->partialFilePath;
                            session->resumeAllowed = false;
                        } else {
                            session->resumeAllowed = true;
                        }
                    } else {
                        mActiveSession.reset();
                        completion = session->completeCb;
                        if (failed) {
                            errorMessage = "Download failed after "
                                + std::to_string(mMaxRetryCount.load(std::memory_order_acquire))
                                + " retries: " + failureDescription;
                        } else {
                            success = true;
                            if (statusCode == 200) {
                                session->resumedBytes = 0;
                            }
                            finalPath = ucf::utilities::FilePathUtils::utf8FromPath(
                                session->partialFilePath);
                        }
                    }
                }

                if (retryAttempt > 0) {
                    const auto delay = getRetryDelay(retryAttempt);
                    UPGRADE_LOG_INFO("Retrying download in " << delay.count()
                                     << "s (attempt " << retryAttempt << "/"
                                     << mMaxRetryCount.load(std::memory_order_acquire) << ")");
                    if (!retryCleanupPath.empty()) {
                        enqueueIo(IoJobKind::Cleanup,
                                  std::chrono::steady_clock::now(),
                                  [retryCleanupPath](std::stop_token) {
                                      std::error_code ec;
                                      std::filesystem::remove(retryCleanupPath, ec);
                                      if (ec) {
                                          UPGRADE_LOG_WARN("Failed to remove HTTP error body: " << ec.message());
                                      }
                                  });
                    }
                    enqueueIo(IoJobKind::Attempt,
                              std::chrono::steady_clock::now() + delay,
                              [this, session](std::stop_token st) {
                                  attemptDownload(session, st);
                              });
                    return;
                }
                if (completion) {
                    if (success) {
                        UPGRADE_LOG_INFO("Download complete: " << finalPath);
                        completion(true, finalPath, model::UpgradeErrorCode::None, "");
                    } else {
                        UPGRADE_LOG_ERROR(errorMessage);
                        completion(false, "", model::UpgradeErrorCode::DownloadFailed,
                                   errorMessage);
                    }
                }
            });
    } catch (const std::exception& ex) {
        {
            std::lock_guard lock(mSessionMutex);
            mOutstandingRequestIds.erase(requestId);
            if (session->requestId == requestId) {
                session->requestId.clear();
            }
        }
        finishSession(session, model::UpgradeErrorCode::NetworkError, ex.what());
        return;
    }

    if (stopToken.stop_requested()
        || mStopRequested.load(std::memory_order_acquire)
        || !isCurrent(*session)) {
        // cancelRequest is synchronous, but this is the I/O worker, never an
        // FSM transition or a thread holding mSessionMutex.
        httpManager->cancelRequest(requestId);
    }
}

void UpgradeDownloadManager::cancelHttpRequest(const std::string& requestId) const
{
    if (requestId.empty()) {
        return;
    }
    if (auto coreFramework = mCoreFramework.lock()) {
        if (auto networkService = coreFramework->getService<INetworkService>().lock()) {
            if (auto httpManager = networkService->getNetworkHttpManager().lock()) {
                httpManager->cancelRequest(requestId);
            }
        }
    }
}

void UpgradeDownloadManager::retireActiveSession(bool preservePartial, bool deletePartial)
{
    std::string requestId;
    std::filesystem::path partialPath;
    std::vector<std::filesystem::path> pathsToDelete;
    {
        std::lock_guard lock(mSessionMutex);
        mGeneration.fetch_add(1, std::memory_order_acq_rel);
        if (mActiveSession) {
            requestId = mActiveSession->requestId;
            partialPath = mActiveSession->partialFilePath;
            if (preservePartial && !partialPath.empty()) {
                mResumePackageInfo = mActiveSession->packageInfo;
            }
            mActiveSession.reset();
        }
        if (preservePartial && !partialPath.empty()) {
            mResumePartialPath = partialPath;
        } else if (!preservePartial) {
            for (const auto& path : {partialPath, mResumePartialPath}) {
                if (!path.empty()
                    && std::find(pathsToDelete.begin(), pathsToDelete.end(), path)
                        == pathsToDelete.end()) {
                    pathsToDelete.push_back(path);
                }
            }
            for (const auto& path : mTrackedFilePaths) {
                if (!path.empty()
                    && std::find(pathsToDelete.begin(), pathsToDelete.end(), path)
                        == pathsToDelete.end()) {
                    pathsToDelete.push_back(path);
                }
            }
            mResumePartialPath.clear();
            mResumePackageInfo.reset();
            mTrackedFilePaths.clear();
        }
    }

    const auto now = std::chrono::steady_clock::now();
    if (!requestId.empty()) {
        enqueueIo(IoJobKind::Cancel, now,
            [this, requestId](std::stop_token) { cancelHttpRequest(requestId); });
    }
    if (deletePartial) {
        for (const auto& path : pathsToDelete) {
            enqueueIo(IoJobKind::Cleanup, now,
            [path](std::stop_token) {
                std::error_code ec;
                std::filesystem::remove(path, ec);
                if (ec) {
                    UPGRADE_LOG_WARN("Failed to remove partial download: " << ec.message());
                }
            });
        }
    }
}

void UpgradeDownloadManager::cancelDownload()
{
    retireActiveSession(true, false);
}

void UpgradeDownloadManager::softReset()
{
    retireActiveSession(true, false);
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager soft reset");
}

void UpgradeDownloadManager::hardReset()
{
    retireActiveSession(false, true);
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager hard reset");
}

void UpgradeDownloadManager::verifyPackage(
    const std::string& filePath,
    const std::string& expectedSha256,
    VerifyCompleteCallback callback)
{
    if (mStopRequested.load(std::memory_order_acquire)) {
        return;
    }
    const auto generation = mGeneration.load(std::memory_order_acquire);
    UPGRADE_LOG_INFO("Verifying package: " << filePath);

    enqueueVerify([this, generation, filePath, expectedSha256,
                   callback = std::move(callback)](std::stop_token st) {
        if (st.stop_requested()
            || mStopRequested.load(std::memory_order_acquire)
            || generation != mGeneration.load(std::memory_order_acquire)) {
            return;
        }

        bool success = false;
        model::UpgradeErrorCode errorCode = model::UpgradeErrorCode::VerifyFailed;
        std::string message;
        try {
            if (!ucf::utilities::FilePathUtils::existsUtf8(filePath)) {
                message = "File not found: " + filePath;
            } else {
                auto actualHash = computeSha256(
                    ucf::utilities::FilePathUtils::pathFromUtf8(filePath), st, generation);
                if (!actualHash) {
                    return;
                }
                if (expectedSha256.empty()) {
                    UPGRADE_LOG_WARN("No expected SHA-256 provided, skipping verification");
                    success = true;
                    errorCode = model::UpgradeErrorCode::None;
                } else if (*actualHash != expectedSha256) {
                    UPGRADE_LOG_ERROR("SHA-256 mismatch: expected=" << expectedSha256
                                      << " actual=" << *actualHash);
                    message = "SHA-256 mismatch: expected " + expectedSha256
                            + ", got " + *actualHash;
                } else {
                    UPGRADE_LOG_INFO("Package verification passed");
                    success = true;
                    errorCode = model::UpgradeErrorCode::None;
                }
            }
        } catch (const std::exception& ex) {
            UPGRADE_LOG_ERROR("Verification exception: " << ex.what());
            message = ex.what();
        }

        // Keep the callback outside the try block: a callback exception must
        // never be mistaken for a hash failure and delivered a second time.
        if (!st.stop_requested()
            && !mStopRequested.load(std::memory_order_acquire)
            && generation == mGeneration.load(std::memory_order_acquire)) {
            callback(success, errorCode, message);
        }
    });
}

std::optional<std::string> UpgradeDownloadManager::computeSha256(
    const std::filesystem::path& filePath,
    std::stop_token stopToken,
    std::uint64_t generation) const
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for SHA-256: " + filePath.string());
    }

    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(
        EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!ctx || EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1) {
        throw std::runtime_error("SHA-256 initialization failed");
    }

    char buffer[8192];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        if (stopToken.stop_requested()
            || mStopRequested.load(std::memory_order_acquire)
            || generation != mGeneration.load(std::memory_order_acquire)) {
            return std::nullopt;
        }
        if (EVP_DigestUpdate(ctx.get(), buffer,
                             static_cast<std::size_t>(file.gcount())) != 1) {
            throw std::runtime_error("SHA-256 update failed");
        }
    }
    if (file.bad()) {
        throw std::runtime_error("Failed to read file for SHA-256: " + filePath.string());
    }
    if (stopToken.stop_requested()
        || mStopRequested.load(std::memory_order_acquire)
        || generation != mGeneration.load(std::memory_order_acquire)) {
        return std::nullopt;
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;
    if (EVP_DigestFinal_ex(ctx.get(), hash, &hashLen) != 1) {
        throw std::runtime_error("SHA-256 finalization failed");
    }

    std::ostringstream oss;
    for (unsigned int i = 0; i < hashLen; ++i) {
        oss << std::hex << std::setfill('0') << std::setw(2)
            << static_cast<int>(hash[i]);
    }
    return oss.str();
}

void UpgradeDownloadManager::setMaxRetryCount(int count)
{
    mMaxRetryCount.store(std::max(0, count), std::memory_order_release);
}

std::chrono::seconds UpgradeDownloadManager::getRetryDelay(int retryAttempt) const
{
    // Clamp the shift so a configured retry count cannot overflow an int.
    return std::chrono::seconds(1 << std::min(retryAttempt, 20));
}

void UpgradeDownloadManager::requestStop()
{
    if (!mStopRequested.exchange(true, std::memory_order_acq_rel)) {
        std::lock_guard lock(mSessionMutex);
        mGeneration.fetch_add(1, std::memory_order_acq_rel);
        mActiveSession.reset();
    }
    mIoWorker.request_stop();
    mVerifyWorker.request_stop();
    mIoCV.notify_all();
    mVerifyCV.notify_all();
}

void UpgradeDownloadManager::cancelOutstandingRequests()
{
    std::vector<std::string> ids;
    {
        std::lock_guard lock(mSessionMutex);
        ids.assign(mOutstandingRequestIds.begin(), mOutstandingRequestIds.end());
    }
    for (const auto& id : ids) {
        cancelHttpRequest(id);
    }
}

void UpgradeDownloadManager::joinWorkers()
{
    if (mIoWorker.joinable()) {
        mIoWorker.join();
    }
    if (mVerifyWorker.joinable()) {
        mVerifyWorker.join();
    }

    // requestStop() drops queued attempts and retries. Finish already queued
    // cleanup after transport cancellation and the I/O worker have completed,
    // so a hard reset immediately followed by shutdown still removes its file.
    std::vector<WorkerJob> cleanupJobs;
    {
        std::lock_guard lock(mIoMutex);
        for (auto& job : mIoJobs) {
            if (job.kind == IoJobKind::Cleanup) {
                cleanupJobs.push_back(std::move(job.run));
            }
        }
        mIoJobs.clear();
    }
    for (auto& cleanup : cleanupJobs) {
        try {
            cleanup(std::stop_token{});
        } catch (const std::exception& ex) {
            UPGRADE_LOG_WARN("Shutdown cleanup failed: " << ex.what());
        }
    }
}

} // namespace ucf::service
