#pragma once

#include <ucf/Services/UpgradeService/UpgradeModel.h>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

/// Responsible for checking a remote server for available upgrades.
class UpgradeCheckManager final
{
public:
    explicit UpgradeCheckManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
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

private:
    model::UpgradeCheckResult parseCheckResponse(const std::string& jsonBody) const;
    bool isNewerVersion(const std::string& current, const std::string& latest) const;

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
    std::string mCheckUrl{"https://api.example.com/v1/upgrade/check"};
    std::chrono::minutes mMinCheckInterval{5};
    std::chrono::steady_clock::time_point mLastCheckTime{};
    std::optional<model::UpgradeCheckResult> mCachedResult;
};

} // namespace ucf::service
