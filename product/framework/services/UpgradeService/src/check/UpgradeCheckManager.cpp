#include "UpgradeCheckManager.h"
#include "../UpgradeServiceLogger.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/NetworkService/Http/INetworkHttpManager.h>
#include <ucf/Services/NetworkService/Model/HttpRestRequest.h>
#include <ucf/Services/NetworkService/Model/HttpRestResponse.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Utilities/JsonUtils/JsonValue.h>

#include <format>

namespace ucf::service {

UpgradeCheckManager::UpgradeCheckManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
{
    UPGRADE_LOG_DEBUG("UpgradeCheckManager created");
}

UpgradeCheckManager::~UpgradeCheckManager()
{
    UPGRADE_LOG_DEBUG("UpgradeCheckManager destroyed");
}

bool UpgradeCheckManager::canCheck() const
{
    if (mLastCheckTime == std::chrono::steady_clock::time_point{}) {
        return true;
    }
    auto elapsed = std::chrono::steady_clock::now() - mLastCheckTime;
    return elapsed >= mMinCheckInterval;
}

std::optional<model::UpgradeCheckResult> UpgradeCheckManager::getCachedResult() const
{
    return mCachedResult;
}

void UpgradeCheckManager::setCheckUrl(const std::string& url)
{
    mCheckUrl = url;
}

void UpgradeCheckManager::setMinCheckInterval(std::chrono::minutes interval)
{
    mMinCheckInterval = interval;
}

void UpgradeCheckManager::reset()
{
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
    // If not user-triggered, respect the minimum check interval
    if (!userTriggered && !canCheck()) {
        UPGRADE_LOG_DEBUG("Check skipped — minimum interval not elapsed");
        if (mCachedResult.has_value()) {
            callback(true, *mCachedResult, model::UpgradeErrorCode::None, "");
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

    // Build request URL with query params
    std::string requestUrl = std::format(
        "{}?version={}&platform={}&arch={}",
        mCheckUrl, currentVersion, platform, arch);

    UPGRADE_LOG_INFO("Checking for upgrade: " << requestUrl);

    auto request = std::make_unique<network::http::HttpRestRequest>(
        network::http::HTTPMethod::GET,
        requestUrl,
        network::http::NetworkHttpHeaders{},
        "",
        30 // timeout seconds
    );

    httpManager->sendHttpRestRequest(*request,
        [this, callback, currentVersion](const network::http::HttpRestResponse& response) {
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

            try {
                auto result = parseCheckResponse(response.getResponseBody());

                // Update cached state
                mLastCheckTime = std::chrono::steady_clock::now();
                mCachedResult = result;

                UPGRADE_LOG_INFO("Check result: hasUpgrade=" << result.hasUpgrade
                                 << (result.hasUpgrade ? (", version=" + result.upgradeInfo.version) : ""));
                callback(true, result, model::UpgradeErrorCode::None, "");
            } catch (const std::exception& ex) {
                UPGRADE_LOG_ERROR("Failed to parse check response: " << ex.what());
                callback(false, {}, model::UpgradeErrorCode::ParseError, ex.what());
            }
        });
}

model::UpgradeCheckResult UpgradeCheckManager::parseCheckResponse(
    const std::string& jsonBody) const
{
    model::UpgradeCheckResult result;
    auto json = ucf::utilities::JsonValue::parse(jsonBody);
    if (json.isNull()) {
        throw std::runtime_error("Failed to parse JSON response");
    }

    result.hasUpgrade = json.get("hasUpgrade").asBool().value_or(false);

    if (result.hasUpgrade) {
        auto latest = json.get("latest");
        auto& info = result.upgradeInfo;

        info.version      = latest.get("version").asString().value_or("");
        info.releaseDate  = latest.get("releaseDate").asString().value_or("");
        info.releaseNotes = latest.get("releaseNotes").asString().value_or("");
        info.mandatory    = latest.get("mandatory").asBool().value_or(false);
        info.minVersion   = latest.get("minVersion").asString().value_or("");

        // Parse platform-specific package
        auto packages = latest.get("package");
        info.package.downloadUrl = packages.get("url").asString().value_or("");
        info.package.sha256      = packages.get("sha256").asString().value_or("");
        info.package.sizeBytes   = packages.get("size").asInt64().value_or(0);
    }

    return result;
}

bool UpgradeCheckManager::isNewerVersion(
    const std::string& /*current*/, const std::string& /*latest*/) const
{
    // Server-side already handles version comparison via the hasUpgrade field.
    // This method is reserved for optional client-side validation.
    return true;
}

} // namespace ucf::service
