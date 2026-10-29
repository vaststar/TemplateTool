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

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <system_error>
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
    model::PackageFormat packageFormat,
    bool userTriggered,
    CheckResultCallback callback)
{
    auto operation = mCallbackGate->tryEnter();
    if (!operation) {
        return;
    }

    // Without a known installation format, no matching installer can be chosen.
    // Reject before the cache path instead of reporting "already up to date".
    if (model::packageFormatName(packageFormat).empty()) {
        callback(false, {}, model::UpgradeErrorCode::UnsupportedInstallation,
                 "Automatic upgrade is unavailable for this installation; download an installer manually");
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
        [this, gate = mCallbackGate, requestId, callback, currentVersion, platformKey, packageFormat](
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
                    response.getResponseBody(), currentVersion, platformKey, packageFormat);
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

    // Cancellation may invoke a completion callback inline; never hold
    // mStateMutex while calling it.
    for (const auto& requestId : requestIds) {
        httpManager->cancelRequest(requestId);
    }
}

model::UpgradeCheckResult UpgradeCheckManager::parseCheckResponse(
    const std::string& jsonBody,
    const std::string& currentVersion,
    const std::string& platformKey,
    model::PackageFormat packageFormat) const
{
    const std::string formatName{model::packageFormatName(packageFormat)};
    if (formatName.empty()) {
        throw std::runtime_error("Unsupported installation package format");
    }

    UPGRADE_LOG_DEBUG("parseCheckResponse jsonBody: " << jsonBody);
    model::UpgradeCheckResult result;
    auto json = ucf::utilities::JsonValue::parse(jsonBody);
    if (!json.isObject()) {
        throw std::runtime_error("Upgrade manifest must be a JSON object");
    }
    const auto schemaVersion = json.get("schemaVersion");
    if (!schemaVersion.isInteger() || schemaVersion.asInt32().value_or(0) != 2) {
        throw std::runtime_error(
            "Unsupported upgrade manifest schema; install the latest version manually");
    }

    auto& info = result.upgradeInfo;
    info.version      = json.get("version").asString().value_or("");
    info.releaseDate  = json.get("releaseDate").asString().value_or("");
    info.releaseNotes = json.get("releaseNotes").asString().value_or("");
    info.mandatory    = json.get("mandatory").asBool().value_or(false);
    info.minVersion   = json.get("minVersion").asString().value_or("");

    const auto candidateVersion = ucf::utilities::Version::parse(info.version);
    if (!candidateVersion) {
        throw std::runtime_error("Manifest version must be a non-empty numeric dotted version");
    }
    const auto installedVersion = ucf::utilities::Version::parse(currentVersion);
    if (!installedVersion) {
        throw std::runtime_error("Invalid current application version: " + currentVersion);
    }
    result.hasUpgrade = candidateVersion->compare(*installedVersion) == std::strong_ordering::greater;
    if (!result.hasUpgrade) {
        return result;
    }

    const auto packages = json.get("packages");
    if (!packages.isObject()) {
        throw std::runtime_error("Manifest packages must be a JSON object");
    }
    const auto platformPackages = packages.get(platformKey);
    if (!platformPackages.isObject()) {
        throw std::runtime_error("Manifest platform packages must be a JSON object: " + platformKey);
    }
    const auto pkg = platformPackages.get(formatName);
    if (!pkg.isObject()) {
        throw std::runtime_error("Manifest installer package is missing or invalid: "
                                 + platformKey + "/" + formatName);
    }

    const auto downloadUrl = pkg.get("url").asString();
    if (!downloadUrl || downloadUrl->empty()
        || std::any_of(downloadUrl->begin(), downloadUrl->end(), [](unsigned char ch) {
               return ch <= 0x20 || ch == 0x7f;
           })) {
        throw std::runtime_error("Package url must be a non-empty string without ASCII whitespace or control characters");
    }

    const auto sha256 = pkg.get("sha256").asString();
    if (!sha256 || sha256->size() != 64
        || !std::all_of(sha256->begin(), sha256->end(), [](unsigned char ch) {
               return (ch >= '0' && ch <= '9')
                   || (ch >= 'a' && ch <= 'f')
                   || (ch >= 'A' && ch <= 'F');
           })) {
        throw std::runtime_error("Package sha256 must contain exactly 64 hexadecimal characters");
    }

    const auto sizeValue = pkg.get("size");
    if (!sizeValue.isNumber()) {
        throw std::runtime_error("Package size must be a positive 64-bit integer");
    }
    // JsonValue accepts integral floating-point values as integers. Parse the
    // serialized number strictly so fractions, exponents and overflow fail.
    const auto sizeText = sizeValue.dump();
    std::int64_t sizeBytes = 0;
    const auto [sizeEnd, sizeError] = std::from_chars(
        sizeText.data(), sizeText.data() + sizeText.size(), sizeBytes);
    if (sizeError != std::errc{} || sizeEnd != sizeText.data() + sizeText.size()
        || sizeBytes <= 0) {
        throw std::runtime_error("Package size must be a positive 64-bit integer");
    }

    info.package.format = packageFormat;
    info.package.downloadUrl = *downloadUrl;
    info.package.sha256 = *sha256;
    std::transform(info.package.sha256.begin(), info.package.sha256.end(),
                   info.package.sha256.begin(), [](unsigned char ch) {
                       return static_cast<char>(std::tolower(ch));
                   });
    info.package.sizeBytes = sizeBytes;
    return result;
}

} // namespace ucf::service
