#include "UpgradeDownloadManager.h"
#include "DownloadFileUtils.h"
#include "PackageVerifier.h"
#include "../AsyncCallbackGate.h"
#include "../UpgradeConstants.h"
#include "../UpgradeServiceLogger.h"

#include <appVersion.h>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/NetworkService/INetworkService.h>
#include <ucf/services/NetworkService/http/INetworkHttpManager.h>
#include <ucf/services/NetworkService/model/HttpDownloadToFileRequest.h>
#include <ucf/services/NetworkService/model/HttpDownloadToFileResponse.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>
#include <ucf/utilities/FilePathUtils/FilePathUtils.h>
#include <ucf/utilities/UUIDUtils/UUIDUtils.h>

#include <algorithm>
#include <exception>
#include <utility>
#include <vector>

namespace ucf::service {

UpgradeDownloadManager::UpgradeDownloadManager(
    ucf::framework::ICoreFrameworkWPtr coreFramework, std::shared_ptr<AsyncCallbackGate> callbackGate)
    : mCoreFramework(std::move(coreFramework))
    , mCallbackGate(std::move(callbackGate))
    , mIoWorker([this](std::stop_token st) { ioWorkerLoop(st); })
    , mVerifyWorker([this](std::stop_token st) { verifyWorkerLoop(st); })
{
    ensureDownloadDirectory();
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager constructed, address: "
                      << this << ", directory: " << ucf::utilities::FilePathUtils::utf8FromPath(mDownloadDir));
}

UpgradeDownloadManager::~UpgradeDownloadManager()
{
    // The owning UpgradeManager normally closes and drains the shared gate
    // before destroying us. Keep this fallback for standalone destruction.
    if (mCallbackGate)
    {
        mCallbackGate->close();
    }
    requestStop();
    cancelOutstandingRequests();
    joinWorkers();
    if (mCallbackGate)
    {
        mCallbackGate->waitForDrain();
    }
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager destroying, address: " << this);
}

void UpgradeDownloadManager::ensureDownloadDirectory()
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        if (auto clientInfo = coreFramework->getService<IClientInfoService>().lock())
        {
            mDownloadDir = ucf::utilities::FilePathUtils::pathFromUtf8(clientInfo->getAppCacheStoragePath()) /
                           upgrade::constants::kDownloadSubDir;
        }
    }
    if (mDownloadDir.empty())
    {
        mDownloadDir =
            std::filesystem::temp_directory_path() / AppInfo::TEMP_DIRECTORY_NAME / upgrade::constants::kDownloadSubDir;
    }
    ucf::utilities::FilePathUtils::createDirectoriesUtf8(ucf::utilities::FilePathUtils::utf8FromPath(mDownloadDir));
}

std::filesystem::path UpgradeDownloadManager::getDownloadDirectory() const
{
    return mDownloadDir;
}

bool UpgradeDownloadManager::isDownloading() const
{
    std::lock_guard lock(mSessionMutex);
    return !mStopRequested.load(std::memory_order_acquire) && static_cast<bool>(mActiveSession);
}

bool UpgradeDownloadManager::hasSufficientSpace(int64_t requiredBytes) const
{
    return upgrade::hasSufficientDownloadSpace(mDownloadDir, requiredBytes);
}

bool UpgradeDownloadManager::enqueueIo(IoJobKind kind, std::chrono::steady_clock::time_point due, WorkerJob job)
{
    {
        std::lock_guard lock(mIoMutex);
        // A callback already admitted by the gate can request a hard reset
        // after shutdown has stopped the worker. Keep its cleanup task for
        // joinWorkers() to run after transport cancellation.
        if (mStopRequested.load(std::memory_order_acquire) && kind != IoJobKind::Cleanup)
        {
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
        if (mStopRequested.load(std::memory_order_acquire))
        {
            return false;
        }
        mVerifyJobs.push_back(std::move(job));
    }
    mVerifyCV.notify_one();
    return true;
}

void UpgradeDownloadManager::ioWorkerLoop(std::stop_token stopToken)
{
    while (!stopToken.stop_requested())
    {
        IoJob job{};
        {
            std::unique_lock lock(mIoMutex);
            for (;;)
            {
                if (stopToken.stop_requested())
                {
                    return;
                }
                if (mIoJobs.empty())
                {
                    mIoCV.wait(lock, stopToken, [this] { return !mIoJobs.empty(); });
                    continue;
                }

                const auto now = std::chrono::steady_clock::now();
                auto ready = mIoJobs.end();
                auto earliest = mIoJobs.front().due;
                for (auto it = mIoJobs.begin(); it != mIoJobs.end(); ++it)
                {
                    earliest = std::min(earliest, it->due);
                    if (it->due > now)
                    {
                        continue;
                    }
                    if (ready == mIoJobs.end() || it->kind < ready->kind ||
                        (it->kind == ready->kind && it->sequence < ready->sequence))
                    {
                        ready = it;
                    }
                }
                if (ready != mIoJobs.end())
                {
                    job = std::move(*ready);
                    mIoJobs.erase(ready);
                    break;
                }

                const auto revision = mIoRevision;
                mIoCV.wait_until(lock, stopToken, earliest, [this, revision] { return mIoRevision != revision; });
            }
        }

        try
        {
            job.run(stopToken);
        }
        catch (const std::exception& ex)
        {
            UPGRADE_LOG_ERROR("Upgrade I/O job failed: " << ex.what());
        }
        catch (...)
        {
            UPGRADE_LOG_ERROR("Upgrade I/O job failed with an unknown exception");
        }
    }
}

void UpgradeDownloadManager::verifyWorkerLoop(std::stop_token stopToken)
{
    while (!stopToken.stop_requested())
    {
        WorkerJob job;
        {
            std::unique_lock lock(mVerifyMutex);
            mVerifyCV.wait(lock, stopToken, [this] { return !mVerifyJobs.empty(); });
            if (stopToken.stop_requested())
            {
                return;
            }
            job = std::move(mVerifyJobs.front());
            mVerifyJobs.pop_front();
        }

        try
        {
            job(stopToken);
        }
        catch (const std::exception& ex)
        {
            UPGRADE_LOG_ERROR("Upgrade verification job failed: " << ex.what());
        }
        catch (...)
        {
            UPGRADE_LOG_ERROR("Upgrade verification job failed with an unknown exception");
        }
    }
}

void UpgradeDownloadManager::findReusableInstaller(
    upgrade::UpgradeOperation operation, const model::PackageInfo& packageInfo, InstallerLookupCallback callback)
{
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || operation.stopToken.stop_requested())
        {
            return;
        }
        mCurrentOperation = operation;
    }
    auto gate = mCallbackGate;
    enqueueIo(IoJobKind::Attempt, std::chrono::steady_clock::now(),
        [this, gate, operation, packageInfo, callback = std::move(callback)](std::stop_token st) {
        auto lease = gate->tryEnter();
        if (!lease || st.stop_requested() || mStopRequested.load(std::memory_order_acquire) ||
            operation.stopToken.stop_requested())
        {
            return;
        }
        const auto installer = findReusableInstallerOnIo(packageInfo, operation);
        if (!st.stop_requested() && !mStopRequested.load(std::memory_order_acquire) &&
            !operation.stopToken.stop_requested() && callback)
        {
            const auto path =
                installer ? std::optional<std::string>{ucf::utilities::FilePathUtils::utf8FromPath(installer->filePath)}
                          : std::nullopt;
            if (path)
            {
                UPGRADE_LOG_INFO("Found cached installer during upgrade check: " << *path);
            }
            else
            {
                UPGRADE_LOG_DEBUG("No matching cached installer for upgrade check");
            }
            callback(path);
        }
    });
}

std::optional<UpgradeDownloadManager::CompletedInstaller> UpgradeDownloadManager::findReusableInstallerOnIo(
    const model::PackageInfo& packageInfo, const upgrade::UpgradeOperation& operation)
{
    std::optional<CompletedInstaller> installer;
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || !isCurrentOperationLocked(operation))
        {
            return std::nullopt;
        }
        installer = mCompletedInstaller;
    }
    const bool fromMemory = installer.has_value();
    if (installer && !upgrade::sameInstallerContent(installer->packageInfo, packageInfo))
    {
        // A manifest probe must not evict another complete installer. Its
        // replacement is owned by the later, explicit download operation.
        return std::nullopt;
    }

    bool reusable = false;
    try
    {
        if (!installer)
        {
            installer = upgrade::readCompletedInstallerRecord(mDownloadDir, packageInfo);
        }
        if (!installer)
        {
            return std::nullopt;
        }
        reusable = upgrade::isReusableInstallerFile(installer->filePath, packageInfo.sizeBytes);
    }
    catch (const std::exception& ex)
    {
        UPGRADE_LOG_WARN("Could not probe cached installer: " << ex.what());
    }
    if (!installer)
    {
        return std::nullopt;
    }

    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || !isCurrentOperationLocked(operation) ||
            (fromMemory && !mCompletedInstaller) ||
            (mCompletedInstaller && (mCompletedInstaller->filePath != installer->filePath ||
                                        !upgrade::sameInstallerContent(mCompletedInstaller->packageInfo, packageInfo))))
        {
            return std::nullopt;
        }
        if (reusable)
        {
            mCompletedInstaller = installer;
            if (std::find(mTrackedFilePaths.begin(), mTrackedFilePaths.end(), installer->filePath) ==
                mTrackedFilePaths.end())
            {
                mTrackedFilePaths.push_back(installer->filePath);
            }
            return installer;
        }
        mCompletedInstaller.reset();
        std::erase(mTrackedFilePaths, installer->filePath);
    }

    const auto unavailablePath = installer->filePath;
    enqueueIo(IoJobKind::Cleanup, std::chrono::steady_clock::now(), [unavailablePath](std::stop_token) {
        upgrade::removeDownloadFile(unavailablePath, "Failed to remove unavailable cached installer");
    });
    syncCompletedInstallerRecord();
    return std::nullopt;
}

void UpgradeDownloadManager::syncCompletedInstallerRecord()
{
    // Cleanup jobs are also drained after stop. Read the slot at execution time
    // rather than capturing an older value from the caller's thread.
    enqueueIo(IoJobKind::Cleanup, std::chrono::steady_clock::now(), [this](std::stop_token) {
        std::optional<CompletedInstaller> installer;
        {
            std::lock_guard lock(mSessionMutex);
            installer = mCompletedInstaller;
        }
        upgrade::writeCompletedInstallerRecord(mDownloadDir, installer);
    });
}

void UpgradeDownloadManager::downloadPackage(upgrade::UpgradeOperation operation, const model::PackageInfo& packageInfo,
    ProgressCallback progressCb, DownloadCompleteCallback completeCb)
{
    if (mStopRequested.load(std::memory_order_acquire) || operation.stopToken.stop_requested())
    {
        return;
    }
    if (model::packageFormatName(packageInfo.format).empty())
    {
        completeCb(false, "", model::UpgradeErrorCode::DownloadFailed, "Unsupported installer format");
        return;
    }
    auto session = std::make_shared<DownloadSession>();
    session->operation = operation;
    session->packageInfo = packageInfo;
    session->progressCb = std::move(progressCb);
    session->completeCb = std::move(completeCb);

    std::string previousRequestId;
    std::filesystem::path staleResumePath;
    std::filesystem::path staleInstallerPath;
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || operation.stopToken.stop_requested())
        {
            return;
        }
        mCurrentOperation = operation;
        mVerifiedInstallerPath.clear();
        if (mCompletedInstaller && !upgrade::sameInstallerContent(mCompletedInstaller->packageInfo, packageInfo))
        {
            staleInstallerPath = mCompletedInstaller->filePath;
            mCompletedInstaller.reset();
            std::erase(mTrackedFilePaths, staleInstallerPath);
        }
        if (mActiveSession)
        {
            previousRequestId = mActiveSession->requestId;
        }
        const bool samePackage = mResumePackageInfo.has_value() && !packageInfo.sha256.empty() &&
                                 mResumePackageInfo->downloadUrl == packageInfo.downloadUrl &&
                                 mResumePackageInfo->sha256 == packageInfo.sha256 &&
                                 mResumePackageInfo->sizeBytes == packageInfo.sizeBytes &&
                                 mResumePackageInfo->format == packageInfo.format;
        if (samePackage)
        {
            session->partialFilePath = std::move(mResumePartialPath);
            session->resumeAllowed = !session->partialFilePath.empty();
        }
        else
        {
            staleResumePath = std::move(mResumePartialPath);
        }
        mResumePartialPath.clear();
        mResumePackageInfo.reset();
        mActiveSession = session;
    }

    const auto now = std::chrono::steady_clock::now();
    if (!previousRequestId.empty())
    {
        enqueueIo(IoJobKind::Cancel, now,
            [this, previousRequestId](std::stop_token) { cancelHttpRequest(previousRequestId); });
    }
    for (const auto& stalePath : {staleResumePath, staleInstallerPath})
    {
        if (!stalePath.empty())
        {
            enqueueIo(IoJobKind::Cleanup, now, [stalePath](std::stop_token) {
                upgrade::removeDownloadFile(stalePath, "Failed to remove stale download");
            });
        }
    }
    if (!staleInstallerPath.empty())
    {
        syncCompletedInstallerRecord();
    }
    enqueueIo(IoJobKind::Attempt, now, [this, session](std::stop_token st) { attemptDownload(session, st); });
}

void UpgradeDownloadManager::finishSession(
    const std::shared_ptr<DownloadSession>& session, model::UpgradeErrorCode errorCode, const std::string& message)
{
    DownloadCompleteCallback callback;
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || !isCurrentOperationLocked(session->operation) ||
            mActiveSession.get() != session.get())
        {
            return;
        }
        mActiveSession.reset();
        callback = session->completeCb;
    }
    if (callback && !session->operation.stopToken.stop_requested())
    {
        callback(false, "", errorCode, message);
    }
}

void UpgradeDownloadManager::attemptDownload(const std::shared_ptr<DownloadSession>& session, std::stop_token stopToken)
{
    if (stopToken.stop_requested() || mStopRequested.load(std::memory_order_acquire) ||
        session->operation.stopToken.stop_requested())
    {
        return;
    }

    bool checkDiskSpace = false;
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || !isCurrentOperationLocked(session->operation) ||
            mActiveSession.get() != session.get())
        {
            return;
        }
        checkDiskSpace = session->currentRetry == 0;
    }

    // Also probe here to defend against the file changing between a manifest
    // lookup and an explicit download request. Never use this as a Range prefix.
    if (const auto installer = findReusableInstallerOnIo(session->packageInfo, session->operation))
    {
        DownloadCompleteCallback completion;
        {
            std::lock_guard lock(mSessionMutex);
            if (mStopRequested.load(std::memory_order_acquire) || !isCurrentOperationLocked(session->operation) ||
                mActiveSession.get() != session.get() || !mCompletedInstaller ||
                mCompletedInstaller->filePath != installer->filePath)
            {
                return;
            }
            mActiveSession.reset();
            completion = session->completeCb;
        }
        if (!stopToken.stop_requested() && !mStopRequested.load(std::memory_order_acquire) &&
            !session->operation.stopToken.stop_requested() && completion)
        {
            const auto filePath = ucf::utilities::FilePathUtils::utf8FromPath(installer->filePath);
            UPGRADE_LOG_INFO("Reusing downloaded installer: " << filePath);
            completion(true, filePath, model::UpgradeErrorCode::None, "");
        }
        return;
    }
    if (stopToken.stop_requested() || mStopRequested.load(std::memory_order_acquire) ||
        session->operation.stopToken.stop_requested())
    {
        return;
    }

    if (checkDiskSpace && !hasSufficientSpace(session->packageInfo.sizeBytes))
    {
        finishSession(session, model::UpgradeErrorCode::DiskSpaceError, "Insufficient disk space");
        return;
    }

    auto coreFramework = mCoreFramework.lock();
    if (!coreFramework)
    {
        finishSession(session, model::UpgradeErrorCode::NetworkError, "CoreFramework unavailable");
        return;
    }
    auto networkService = coreFramework->getService<INetworkService>().lock();
    if (!networkService)
    {
        finishSession(session, model::UpgradeErrorCode::NetworkError, "NetworkService unavailable");
        return;
    }
    auto httpManager = networkService->getNetworkHttpManager().lock();
    if (!httpManager)
    {
        finishSession(session, model::UpgradeErrorCode::NetworkError, "HttpManager unavailable");
        return;
    }

    std::filesystem::path partialPath;
    bool resumeAllowed = false;
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || !isCurrentOperationLocked(session->operation) ||
            mActiveSession.get() != session.get())
        {
            return;
        }
        if (session->partialFilePath.empty())
        {
            // URL query strings do not belong in local filenames. A fresh name also
            // prevents a later download from overwriting an installer already opened.
            const auto fileName = "installer-" + ucf::utilities::UUIDUtils::generateUUID() + "." +
                                  std::string(model::packageFormatName(session->packageInfo.format));
            session->partialFilePath = mDownloadDir / fileName;
        }
        if (std::find(mTrackedFilePaths.begin(), mTrackedFilePaths.end(), session->partialFilePath) ==
            mTrackedFilePaths.end())
        {
            mTrackedFilePaths.push_back(session->partialFilePath);
        }
        partialPath = session->partialFilePath;
        resumeAllowed = session->resumeAllowed;
    }

    std::int64_t resumedBytes = 0;
    if (resumeAllowed)
    {
        std::error_code ec;
        if (std::filesystem::exists(partialPath, ec) && !ec)
        {
            auto size = std::filesystem::file_size(partialPath, ec);
            if (!ec)
            {
                resumedBytes = static_cast<std::int64_t>(size);
            }
        }
    }

    network::http::NetworkHttpHeaders headers;
    if (resumedBytes > 0)
    {
        headers.emplace_back("Range", "bytes=" + std::to_string(resumedBytes) + "-");
        UPGRADE_LOG_INFO("Resuming download from byte " << resumedBytes);
    }

    UPGRADE_LOG_INFO("Starting download (operation=" << session->operation.id
                                                     << "): " << session->packageInfo.downloadUrl << " → "
                                                     << ucf::utilities::FilePathUtils::utf8FromPath(partialPath));

    auto request = std::make_unique<network::http::HttpDownloadToFileRequest>(
        session->packageInfo.downloadUrl, headers, 300, ucf::utilities::FilePathUtils::utf8FromPath(partialPath));
    const std::string requestId = request->getRequestId();

    // Publish the ID before sending. If shutdown cancels it before the HTTP
    // layer enqueues it, the post-send check below cancels it again.
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || !isCurrentOperationLocked(session->operation) ||
            mActiveSession.get() != session.get())
        {
            return;
        }
        session->resumedBytes = resumedBytes;
        session->requestId = requestId;
        mOutstandingRequestIds.insert(requestId);
    }

    auto gate = mCallbackGate;
    try
    {
        httpManager->downloadContentToFile(
            *request, [this, gate, session, requestId](const network::http::HttpDownloadToFileResponse& response) {
            auto lease = gate->tryEnter();
            if (!lease)
            {
                return;
            }

            if (response.isFinished())
            {
                handleDownloadCompleted(session, requestId, response);
            }
            else
            {
                handleDownloadProgress(session, requestId, response);
            }
        });
    }
    catch (const std::exception& ex)
    {
        {
            std::lock_guard lock(mSessionMutex);
            mOutstandingRequestIds.erase(requestId);
            if (session->requestId == requestId)
            {
                session->requestId.clear();
            }
        }
        finishSession(session, model::UpgradeErrorCode::NetworkError, ex.what());
        return;
    }

    if (stopToken.stop_requested() || mStopRequested.load(std::memory_order_acquire) ||
        session->operation.stopToken.stop_requested())
    {
        // Cancellation may invoke completion inline. This is the I/O worker,
        // outside an FSM transition and without mSessionMutex held.
        httpManager->cancelRequest(requestId);
    }
}

void UpgradeDownloadManager::handleDownloadProgress(const std::shared_ptr<DownloadSession>& session,
    const std::string& requestId, const network::http::HttpDownloadToFileResponse& response)
{
    ProgressCallback progress;
    std::int64_t current = 0;
    std::int64_t total = 0;
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || !isCurrentOperationLocked(session->operation) ||
            mActiveSession.get() != session.get() || session->requestId != requestId)
        {
            return;
        }
        // Redirect/error response bodies are not package
        // progress; the network layer may follow redirects.
        const int statusCode = response.getHttpResponseCode();
        if (statusCode != 200 && statusCode != 206)
        {
            return;
        }
        // A server may ignore Range and send the whole file.
        // The HTTP handler truncates in that case, so progress
        // must start at zero as well.
        if (statusCode == 200)
        {
            session->resumedBytes = 0;
        }
        current = static_cast<std::int64_t>(response.getCurrentSize()) + session->resumedBytes;
        total = static_cast<std::int64_t>(response.getTotalSize()) + session->resumedBytes;
        const auto now = std::chrono::steady_clock::now();
        const bool complete = total > 0 && current >= total;
        if (complete || now - session->lastProgressTime >= std::chrono::milliseconds(200))
        {
            session->lastProgressTime = now;
            progress = session->progressCb;
        }
    }
    if (progress && !session->operation.stopToken.stop_requested())
    {
        progress(current, total);
    }
}

void UpgradeDownloadManager::handleDownloadCompleted(const std::shared_ptr<DownloadSession>& session,
    const std::string& requestId, const network::http::HttpDownloadToFileResponse& response)
{
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
    std::string failureDescription =
        errorData.has_value() ? errorData->errorDescription : "HTTP " + std::to_string(statusCode);

    // The file handler has closed its stream before the terminal
    // callback. Check the saved file, including an empty 206 body
    // for which the handler never had to open the file.
    bool invalidFileSize = false;
    if (!transportFailed)
    {
        if (response.getCurrentSize() == 0)
        {
            invalidFileSize = true;
            failureDescription = "Download response has no package bytes";
        }
        std::filesystem::path downloadedPath;
        {
            std::lock_guard lock(mSessionMutex);
            downloadedPath = session->partialFilePath;
        }
        if (const auto sizeError = upgrade::validateDownloadedFileSize(downloadedPath, session->packageInfo.sizeBytes))
        {
            invalidFileSize = true;
            failureDescription = *sizeError;
        }
    }
    const bool failed = transportFailed || invalidFileSize;
    {
        std::lock_guard lock(mSessionMutex);
        // Consume a terminal response once. A duplicate completion
        // must not schedule another retry or complete twice.
        if (mOutstandingRequestIds.erase(requestId) == 0 || session->requestId != requestId)
        {
            return;
        }
        session->requestId.clear();
        if (mStopRequested.load(std::memory_order_acquire) || !isCurrentOperationLocked(session->operation) ||
            mActiveSession.get() != session.get())
        {
            return;
        }

        if (failed && session->currentRetry < mMaxRetryCount.load(std::memory_order_acquire))
        {
            retryAttempt = ++session->currentRetry;
            // An HTTP error body is not part of the package and
            // must not become the prefix of the next Range request.
            const bool restartFromZero =
                (httpError && statusCode >= 300) || invalidFileSize || session->packageInfo.sha256.empty() ||
                (errorData.has_value() && errorData->errorType == network::http::ResponseErrorType::OtherError);
            if (restartFromZero)
            {
                retryCleanupPath = session->partialFilePath;
                session->resumeAllowed = false;
            }
            else
            {
                session->resumeAllowed = true;
            }
        }
        else
        {
            mActiveSession.reset();
            completion = session->completeCb;
            if (failed)
            {
                errorMessage = "Download failed after " +
                               std::to_string(mMaxRetryCount.load(std::memory_order_acquire)) +
                               " retries: " + failureDescription;
            }
            else
            {
                success = true;
                if (statusCode == 200)
                {
                    session->resumedBytes = 0;
                }
                finalPath = ucf::utilities::FilePathUtils::utf8FromPath(session->partialFilePath);
            }
        }
    }

    if (retryAttempt > 0)
    {
        const auto delay = getRetryDelay(retryAttempt);
        UPGRADE_LOG_INFO("Retrying download in " << delay.count() << "s (attempt " << retryAttempt << "/"
                                                 << mMaxRetryCount.load(std::memory_order_acquire) << ")");
        if (!retryCleanupPath.empty())
        {
            enqueueIo(IoJobKind::Cleanup, std::chrono::steady_clock::now(), [retryCleanupPath](std::stop_token) {
                upgrade::removeDownloadFile(retryCleanupPath, "Failed to remove HTTP error body");
            });
        }
        enqueueIo(IoJobKind::Attempt, std::chrono::steady_clock::now() + delay,
            [this, session](std::stop_token st) { attemptDownload(session, st); });
        return;
    }
    if (completion && !session->operation.stopToken.stop_requested())
    {
        if (success)
        {
            UPGRADE_LOG_INFO("Download complete: " << finalPath);
            completion(true, finalPath, model::UpgradeErrorCode::None, "");
        }
        else
        {
            UPGRADE_LOG_ERROR(errorMessage);
            completion(false, "", model::UpgradeErrorCode::DownloadFailed, errorMessage);
        }
    }
}

void UpgradeDownloadManager::cancelHttpRequest(const std::string& requestId) const
{
    if (requestId.empty())
    {
        return;
    }
    if (auto coreFramework = mCoreFramework.lock())
    {
        if (auto networkService = coreFramework->getService<INetworkService>().lock())
        {
            if (auto httpManager = networkService->getNetworkHttpManager().lock())
            {
                httpManager->cancelRequest(requestId);
            }
        }
    }
}

void UpgradeDownloadManager::retireActiveSession(PartialFilePolicy policy)
{
    std::string requestId;
    std::filesystem::path partialPath;
    std::vector<std::filesystem::path> pathsToDelete;
    {
        std::lock_guard lock(mSessionMutex);
        mCurrentOperation.reset();
        mVerifiedInstallerPath.clear();
        if (mActiveSession)
        {
            requestId = mActiveSession->requestId;
            partialPath = mActiveSession->partialFilePath;
            if (policy == PartialFilePolicy::KeepForResume && !partialPath.empty())
            {
                mResumePackageInfo = mActiveSession->packageInfo;
            }
            mActiveSession.reset();
        }
        if (policy == PartialFilePolicy::KeepForResume && !partialPath.empty())
        {
            mResumePartialPath = partialPath;
        }
        else if (policy == PartialFilePolicy::Remove)
        {
            const auto cachedPath = mCompletedInstaller ? mCompletedInstaller->filePath : std::filesystem::path{};
            for (const auto& path : {partialPath, mResumePartialPath})
            {
                if (!path.empty() && path != cachedPath &&
                    std::find(pathsToDelete.begin(), pathsToDelete.end(), path) == pathsToDelete.end())
                {
                    pathsToDelete.push_back(path);
                }
            }
            for (const auto& path : mTrackedFilePaths)
            {
                if (!path.empty() && path != cachedPath &&
                    std::find(pathsToDelete.begin(), pathsToDelete.end(), path) == pathsToDelete.end())
                {
                    pathsToDelete.push_back(path);
                }
            }
            mResumePartialPath.clear();
            mResumePackageInfo.reset();
            std::erase_if(mTrackedFilePaths,
                [&cachedPath](const auto& path) { return cachedPath.empty() || path != cachedPath; });
        }
    }

    const auto now = std::chrono::steady_clock::now();
    if (!requestId.empty())
    {
        enqueueIo(IoJobKind::Cancel, now, [this, requestId](std::stop_token) { cancelHttpRequest(requestId); });
    }
    if (policy == PartialFilePolicy::Remove)
    {
        for (const auto& path : pathsToDelete)
        {
            enqueueIo(IoJobKind::Cleanup, now,
                [path](std::stop_token) { upgrade::removeDownloadFile(path, "Failed to remove partial download"); });
        }
    }
}

void UpgradeDownloadManager::cancelDownload()
{
    retireActiveSession(PartialFilePolicy::KeepForResume);
}

void UpgradeDownloadManager::softReset()
{
    retireActiveSession(PartialFilePolicy::KeepForResume);
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager soft reset");
}

void UpgradeDownloadManager::hardReset()
{
    retireActiveSession(PartialFilePolicy::Remove);
    UPGRADE_LOG_DEBUG("UpgradeDownloadManager hard reset (retain verified installer)");
}

void UpgradeDownloadManager::verifyPackage(upgrade::UpgradeOperation operation, const std::string& filePath,
    const model::PackageInfo& packageInfo, VerifyCompleteCallback callback)
{
    if (mStopRequested.load(std::memory_order_acquire) || operation.stopToken.stop_requested())
    {
        return;
    }
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || operation.stopToken.stop_requested())
        {
            return;
        }
        mCurrentOperation = operation;
        mVerifiedInstallerPath.clear();
    }
    UPGRADE_LOG_INFO("Verifying package: " << filePath);

    enqueueVerify([this, operation, filePath, packageInfo, callback = std::move(callback)](std::stop_token st) {
        if (st.stop_requested() || mStopRequested.load(std::memory_order_acquire) ||
            operation.stopToken.stop_requested())
        {
            return;
        }

        const auto result = upgrade::verifyPackageFile(filePath, packageInfo.sha256, [this, st, operation] {
            return st.stop_requested() || mStopRequested.load(std::memory_order_acquire) ||
                   operation.stopToken.stop_requested();
        });
        if (result.status == upgrade::VerificationStatus::Cancelled)
        {
            return;
        }
        bool success = result.status == upgrade::VerificationStatus::Passed;
        auto errorCode = success ? model::UpgradeErrorCode::None : model::UpgradeErrorCode::VerifyFailed;
        std::string message = result.message;

        bool recordChanged = false;
        std::filesystem::path invalidCachedPath;
        {
            std::lock_guard lock(mSessionMutex);
            if (st.stop_requested() || mStopRequested.load(std::memory_order_acquire) ||
                !isCurrentOperationLocked(operation))
            {
                return;
            }
            const auto verifiedPath = ucf::utilities::FilePathUtils::pathFromUtf8(filePath);
            if (success)
            {
                if (mActiveSession || std::find(mTrackedFilePaths.begin(), mTrackedFilePaths.end(), verifiedPath) ==
                                          mTrackedFilePaths.end())
                {
                    success = false;
                    errorCode = model::UpgradeErrorCode::VerifyFailed;
                    message = "Installer is not a completed download owned by this manager";
                }
                else
                {
                    mVerifiedInstallerPath = verifiedPath;
                    mCompletedInstaller = CompletedInstaller{packageInfo, verifiedPath};
                    recordChanged = true;
                }
            }
            if (!success && mCompletedInstaller && mCompletedInstaller->filePath == verifiedPath)
            {
                invalidCachedPath = mCompletedInstaller->filePath;
                mCompletedInstaller.reset();
                std::erase(mTrackedFilePaths, invalidCachedPath);
                recordChanged = true;
            }
        }
        if (!invalidCachedPath.empty())
        {
            enqueueIo(IoJobKind::Cleanup, std::chrono::steady_clock::now(), [invalidCachedPath](std::stop_token) {
                upgrade::removeDownloadFile(invalidCachedPath, "Failed to remove invalid cached installer");
            });
        }
        if (recordChanged)
        {
            syncCompletedInstallerRecord();
        }

        // Keep the callback outside the verifier and session mutex: the FSM
        // may reset the manager or release the verified installer from here.
        if (!st.stop_requested() && !mStopRequested.load(std::memory_order_acquire) &&
            !operation.stopToken.stop_requested())
        {
            callback(success, errorCode, message);
        }
    });
}

bool UpgradeDownloadManager::releaseInstaller(const std::string& filePath)
{
    const auto installerPath = ucf::utilities::FilePathUtils::pathFromUtf8(filePath);
    {
        std::lock_guard lock(mSessionMutex);
        if (mStopRequested.load(std::memory_order_acquire) || mVerifiedInstallerPath.empty() ||
            mVerifiedInstallerPath != installerPath)
        {
            return false;
        }
        const auto tracked = std::find(mTrackedFilePaths.begin(), mTrackedFilePaths.end(), installerPath);
        if (tracked == mTrackedFilePaths.end())
        {
            return false;
        }

        mTrackedFilePaths.erase(tracked);
        if (mCompletedInstaller && mCompletedInstaller->filePath == installerPath)
        {
            mCompletedInstaller.reset();
        }
        if (mResumePartialPath == installerPath)
        {
            mResumePartialPath.clear();
            mResumePackageInfo.reset();
        }
        mVerifiedInstallerPath.clear();
    }
    syncCompletedInstallerRecord();
    return true;
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
    if (!mStopRequested.exchange(true, std::memory_order_acq_rel))
    {
        std::lock_guard lock(mSessionMutex);
        mCurrentOperation.reset();
        mVerifiedInstallerPath.clear();
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
    for (const auto& id : ids)
    {
        cancelHttpRequest(id);
    }
}

void UpgradeDownloadManager::joinWorkers()
{
    if (mIoWorker.joinable())
    {
        mIoWorker.join();
    }
    if (mVerifyWorker.joinable())
    {
        mVerifyWorker.join();
    }

    // requestStop() drops queued attempts and retries. Finish already queued
    // cleanup and record sync after transport cancellation and both workers
    // have completed, so shutdown also persists the latest cache slot.
    std::vector<WorkerJob> cleanupJobs;
    {
        std::lock_guard lock(mIoMutex);
        for (auto& job : mIoJobs)
        {
            if (job.kind == IoJobKind::Cleanup)
            {
                cleanupJobs.push_back(std::move(job.run));
            }
        }
        mIoJobs.clear();
    }
    for (auto& cleanup : cleanupJobs)
    {
        try
        {
            cleanup(std::stop_token{});
        }
        catch (const std::exception& ex)
        {
            UPGRADE_LOG_WARN("Shutdown cleanup failed: " << ex.what());
        }
    }
}

} // namespace ucf::service
