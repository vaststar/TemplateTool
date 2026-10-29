#pragma once

#include <memory>
#include <optional>

#include <ucf/utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/services/ServiceDeclaration/IService.h>
#include <ucf/services/UpgradeService/UpgradeModel.h>
#include <ucf/services/UpgradeService/IUpgradeServiceCallback.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class IUpgradeService
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

    /// Ask the presentation layer to open the verified installer.
    virtual void openInstaller() = 0;

    /// Report desktop acceptance of the open request, not installation success.
    virtual void reportInstallerOpenResult(bool success) = 0;

    /// Cancel an in-progress download
    virtual void cancelDownload() = 0;

    /// Dismiss the upgrade prompt (return to idle)
    virtual void dismissUpgrade() = 0;

    // ── Queries ──

    /// Get the current upgrade state
    [[nodiscard]] virtual model::UpgradeState getUpgradeState() const = 0;

    /// Get available upgrade info (valid after check succeeds)
    [[nodiscard]] virtual std::optional<model::UpgradeInfo> getAvailableUpgrade() const = 0;
};

} // namespace ucf::service
