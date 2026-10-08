#pragma once

#include <ucf/services/UpgradeService/UpgradeModel.h>
#include "../UpgradeConstants.h"
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class AsyncCallbackGate;

/// Responsible for checking a remote server for available upgrades.
class UpgradeCheckManager final
{
public:
    UpgradeCheckManager(ucf::framework::ICoreFrameworkWPtr coreFramework,
                        std::shared_ptr<AsyncCallbackGate> callbackGate);
    ~UpgradeCheckManager();

    UpgradeCheckManager(const UpgradeCheckManager&) = delete;
    UpgradeCheckManager& operator=(const UpgradeCheckManager&) = delete;

    /// Result callback type
    using CheckResultCallback = std::function<void(
        bool success,
        const model::UpgradeCheckResult& result,
        model::UpgradeErrorCode errorCode,
        const std::string& errorMessage)>;

    /// Issue an upgrade check (async — result delivered via callback)
    void checkForUpgrade(const std::string& currentVersion,
                         const std::string& platform,
                         const std::string& arch,
                         bool userTriggered,
                         CheckResultCallback callback);

    /// Whether the minimum check interval has elapsed (ignored when userTriggered)
    [[nodiscard]] bool canCheck() const;

    /// Get cached result from last successful check
    [[nodiscard]] std::optional<model::UpgradeCheckResult> getCachedResult() const;

    /// Set the upgrade check URL
    void setCheckUrl(const std::string& url);

    /// Set minimum interval between automatic checks (default: 5 min)
    void setMinCheckInterval(std::chrono::minutes interval);

    /// Reset cached result (not the last-check timestamp)
    void reset();

    /// Cancel requests currently known to be in flight. Call after closing the
    /// callback gate, and repeat after admitted submissions have drained.
    void cancelOutstandingRequests();

private:
    bool canCheckLocked(std::chrono::steady_clock::time_point now) const;

    model::UpgradeCheckResult parseCheckResponse(
        const std::string& jsonBody,
        const std::string& currentVersion,
        const std::string& platformKey) const;

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
    std::shared_ptr<AsyncCallbackGate> mCallbackGate;

    mutable std::mutex mStateMutex;
    std::string mCheckUrl{upgrade::constants::kDefaultManifestUrl};
    std::chrono::minutes mMinCheckInterval{5};
    std::chrono::steady_clock::time_point mLastCheckTime{};
    std::optional<model::UpgradeCheckResult> mCachedResult;
    std::unordered_set<std::string> mOutstandingRequestIds;
};

} // namespace ucf::service
