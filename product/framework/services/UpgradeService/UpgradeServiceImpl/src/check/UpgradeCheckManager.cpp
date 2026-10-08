#include "UpgradeCheckManager.h"
#include "../AsyncCallbackGate.h"
#include "../UpgradeServiceLogger.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/NetworkService/INetworkService.h>
#include <ucf/services/NetworkService/http/INetworkHttpManager.h>
#include <ucf/services/NetworkService/model/HttpRestRequest.h>
#include <ucf/services/NetworkService/model/HttpRestResponse.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>
#include <ucf/utilities/JsonUtils/JsonValue.h>
#include <ucf/utilities/VersionUtils/Version.h>

#include <format>
#include <utility>
#include <vector>

namespace ucf::service {

UpgradeCheckManager::UpgradeCheckManager(
    ucf::framework::ICoreFrameworkWPtr coreFramework,
    std::shared_ptr<AsyncCallbackGate> callbackGate)
    : mCoreFramework(coreFramework)
    , mCallbackGate(std::move(callbackGate))
{
    UPGRADE_LOG_DEBUG("UpgradeCheckManager constructed, address: " << this);
}

UpgradeCheckManager::~UpgradeCheckManager()
{
    UPGRADE_LOG_DEBUG("UpgradeCheckManager destroying, address: " << this);
}

bool UpgradeCheckManager::canCheck() const
{
    std::lock_guard lock(mStateMutex);
    return canCheckLocked(std::chrono::steady_clock::now());
}

bool UpgradeCheckManager::canCheckLocked(std::chrono::steady_clock::time_point now) const
{
    if (mLastCheckTime == std::chrono::steady_clock::time_point{}) {
        return true;
    }
    auto elapsed = now - mLastCheckTime;
    return elapsed >= mMinCheckInterval;
}

std::optional<model::UpgradeCheckResult> UpgradeCheckManager::getCachedResult() const
{
    std::lock_guard lock(mStateMutex);
    return mCachedResult;
}

void UpgradeCheckManager::setCheckUrl(const std::string& url)
{
    std::lock_guard lock(mStateMutex);
    mCheckUrl = url;
}

void UpgradeCheckManager::setMinCheckInterval(std::chrono::minutes interval)
{
    std::lock_guard lock(mStateMutex);
    mMinCheckInterval = interval;
}

void UpgradeCheckManager::reset()
{
    std::lock_guard lock(mStateMutex);
    mCachedResult.reset();
    // Note: mLastCheckTime is intentionally NOT reset —
    // check interval should survive across upgrade cycles.
}

void UpgradeCheckManager::checkForUpgrade(
    const std::string& currentVersion,
    const std::string& platform,
    const std::string& arch,
    bool userTriggered,
    CheckResultCallback callback)
{
    auto operation = mCallbackGate->tryEnter();
    if (!operation) {
        return;
    }

    // If not user-triggered, respect the minimum check interval
    bool skipCheck = false;
    std::optional<model::UpgradeCheckResult> cachedResult;
    std::string checkUrl;
    {
        std::lock_guard lock(mStateMutex);
        skipCheck = !userTriggered && !canCheckLocked(std::chrono::steady_clock::now());
        if (skipCheck) {
            cachedResult = mCachedResult;
        }
        checkUrl = mCheckUrl;
    }
    if (skipCheck) {
        UPGRADE_LOG_DEBUG("Check skipped — minimum interval not elapsed");
        if (cachedResult.has_value()) {
            callback(true, *cachedResult, model::UpgradeErrorCode::None, "");
        } else {
            callback(true, model::UpgradeCheckResult{false, {}}, model::UpgradeErrorCode::None, "");
        }
        return;
    }

    auto coreFramework = mCoreFramework.lock();
    if (!coreFramework) {
        UPGRADE_LOG_ERROR("CoreFramework is null");
        callback(false, {}, model::UpgradeErrorCode::NetworkError, "CoreFramework unavailable");
        return;
    }

    auto networkService = coreFramework->getService<INetworkService>().lock();
    if (!networkService) {
        UPGRADE_LOG_ERROR("NetworkService is null");
        callback(false, {}, model::UpgradeErrorCode::NetworkError, "NetworkService unavailable");
        return;
    }

    auto httpManager = networkService->getNetworkHttpManager().lock();
    if (!httpManager) {
        UPGRADE_LOG_ERROR("NetworkHttpManager is null");
        callback(false, {}, model::UpgradeErrorCode::NetworkError, "HttpManager unavailable");
        return;
    }

    // The manifest URL points directly to upgrade-manifest.json in the
    // latest GitHub Release.  No query params needed — the manifest contains
    // all platforms; the client picks its own after download.
    UPGRADE_LOG_INFO("Checking for upgrade: " << checkUrl);

    auto request = std::make_unique<network::http::HttpRestRequest>(
        network::http::HTTPMethod::GET,
        checkUrl,
        network::http::NetworkHttpHeaders{},
        "",
        30 // timeout seconds
    );

    // Capture platform key for manifest lookup (e.g. "windows-x64")
    std::string platformKey = platform + "-" + arch;
    const std::string requestId = request->getRequestId();

    {
        std::lock_guard lock(mStateMutex);
        mOutstandingRequestIds.insert(requestId);
    }

    httpManager->sendHttpRestRequest(*request,
        [this, gate = mCallbackGate, requestId, callback, currentVersion, platformKey](
            const network::http::HttpRestResponse& response) {
            auto lease = gate->tryEnter();
            if (!lease) {
                return;
            }
            {
                std::lock_guard lock(mStateMutex);
                // A terminal response is handled only once for each request.
                if (mOutstandingRequestIds.erase(requestId) == 0) {
                    return;
                }
            }

            auto errorData = response.getErrorData();
            if (errorData.has_value()) {
                UPGRADE_LOG_ERROR("Check request failed: " << errorData->errorDescription);
                callback(false, {}, model::UpgradeErrorCode::NetworkError, errorData->errorDescription);
                return;
            }

            int statusCode = response.getHttpResponseCode();
            if (statusCode < 200 || statusCode >= 300) {
                UPGRADE_LOG_ERROR("Check request returned HTTP " << statusCode);
                callback(false, {}, model::UpgradeErrorCode::ServerError,
                         std::format("HTTP {}", statusCode));
                return;
            }

            model::UpgradeCheckResult result;
            try {
                result = parseCheckResponse(
                    response.getResponseBody(), currentVersion, platformKey);
            } catch (const std::exception& ex) {
                UPGRADE_LOG_ERROR("Failed to parse check response: " << ex.what());
                callback(false, {}, model::UpgradeErrorCode::ParseError, ex.what());
                return;
            }

            {
                std::lock_guard lock(mStateMutex);
                mLastCheckTime = std::chrono::steady_clock::now();
                mCachedResult = result;
            }

            UPGRADE_LOG_INFO("Check result: hasUpgrade=" << result.hasUpgrade
                             << (result.hasUpgrade ? (", version=" + result.upgradeInfo.version) : ""));
            callback(true, result, model::UpgradeErrorCode::None, "");
        });

    // close() may have raced with request submission. shutdown() will also
    // sweep registered IDs after admitted work drains, but cancel promptly here.
    if (mCallbackGate->isClosed()) {
        httpManager->cancelRequest(requestId);
    }
}

void UpgradeCheckManager::cancelOutstandingRequests()
{
    std::vector<std::string> requestIds;
    {
        std::lock_guard lock(mStateMutex);
        requestIds.assign(mOutstandingRequestIds.begin(), mOutstandingRequestIds.end());
        mOutstandingRequestIds.clear();
    }
    if (requestIds.empty()) {
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

    // Cancellation invokes a completion callback synchronously; never hold
    // mStateMutex while calling it.
    for (const auto& requestId : requestIds) {
        httpManager->cancelRequest(requestId);
    }
}

model::UpgradeCheckResult UpgradeCheckManager::parseCheckResponse(
    const std::string& jsonBody,
    const std::string& currentVersion,
    const std::string& platformKey) const
{
    UPGRADE_LOG_DEBUG("parseCheckResponse jsonBody: " << jsonBody);
    model::UpgradeCheckResult result;
    auto json = ucf::utilities::JsonValue::parse(jsonBody);
    if (json.isNull()) {
        throw std::runtime_error("Failed to parse JSON response");
    }

    auto& info = result.upgradeInfo;
    info.version      = json.get("version").asString().value_or("");
    info.releaseDate  = json.get("releaseDate").asString().value_or("");
    info.releaseNotes = json.get("releaseNotes").asString().value_or("");
    info.mandatory    = json.get("mandatory").asBool().value_or(false);
    info.minVersion   = json.get("minVersion").asString().value_or("");

    // Check if the manifest version is newer than current. Malformed
    // version strings are treated as "not newer" — VersionUtils returns
    // false in that case rather than throwing.
    result.hasUpgrade = ucf::utilities::Version::isNewer(info.version, currentVersion);

    if (result.hasUpgrade) {
        // Look up platform-specific package (e.g. "windows-x64")
        auto packages = json.get("packages");
        auto pkg = packages.get(platformKey);

        if (pkg.isNull()) {
            UPGRADE_LOG_WARN("No package found for platform: " << platformKey);
            result.hasUpgrade = false;
            return result;
        }

        info.package.downloadUrl = pkg.get("url").asString().value_or("");
        info.package.sha256      = pkg.get("sha256").asString().value_or("");
        info.package.sizeBytes   = pkg.get("size").asInt64().value_or(0);
    }

    return result;
}

} // namespace ucf::service
