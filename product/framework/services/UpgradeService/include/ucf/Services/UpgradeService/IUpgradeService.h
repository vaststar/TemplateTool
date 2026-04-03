#pragma once

#include <memory>
#include <optional>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/UpgradeService/UpgradeModel.h>
#include <ucf/Services/UpgradeService/IUpgradeServiceCallback.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class SERVICE_EXPORT IUpgradeService
    : public IService
    , public virtual ucf::utilities::INotificationHelper<IUpgradeServiceCallback>
{
public:
    IUpgradeService() = default;
    IUpgradeService(const IUpgradeService&) = delete;
    IUpgradeService(IUpgradeService&&) = delete;
    IUpgradeService& operator=(const IUpgradeService&) = delete;
    IUpgradeService& operator=(IUpgradeService&&) = delete;
    virtual ~IUpgradeService() = default;

public:
    // ── Actions ──

    /// Check for available upgrade (userTriggered=true ignores check interval)
    virtual void checkForUpgrade(bool userTriggered = false) = 0;

    /// Start downloading the available upgrade package
    virtual void downloadUpgrade() = 0;

    /// Launch updater process and quit the application
    virtual void installAndRestart() = 0;

    /// Cancel an in-progress download
    virtual void cancelDownload() = 0;

    /// Dismiss the upgrade prompt (return to idle)
    virtual void remindLater() = 0;

    // ── Queries ──

    /// Get the current upgrade state
    [[nodiscard]] virtual model::UpgradeState getUpgradeState() const = 0;

    /// Get available upgrade info (valid after check succeeds)
    [[nodiscard]] virtual std::optional<model::UpgradeInfo> getAvailableUpgrade() const = 0;

public:
    static std::shared_ptr<IUpgradeService> createInstance(
        ucf::framework::ICoreFrameworkWPtr coreFramework);
};

} // namespace ucf::service
