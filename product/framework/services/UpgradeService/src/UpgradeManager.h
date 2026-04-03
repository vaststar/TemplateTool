#pragma once

#include <ucf/Services/UpgradeService/UpgradeModel.h>
#include <ucf/Services/UpgradeService/IUpgradeServiceCallback.h>
#include "fsm/UpgradeStates.h"

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class UpgradeCheckManager;
class UpgradeDownloadManager;
class UpgradeInstallManager;

/// Coordinates the full upgrade lifecycle:
///   - Owns the FSM, sub-managers, and persistence layer.
///   - Receives Listener calls and forwards them to the Service for fireNotification.
class UpgradeManager final
{
public:
    /// Internal listener interface — implemented by UpgradeService via private inheritance
    struct Listener {
        virtual ~Listener() = default;
        virtual void onUpgradeStateChanged(model::UpgradeState state) = 0;
        virtual void onUpgradeCheckCompleted(const model::UpgradeCheckResult& result) = 0;
        virtual void onDownloadProgressChanged(int64_t currentBytes, int64_t totalBytes) = 0;
        virtual void onUpgradeError(model::UpgradeErrorCode code, const std::string& msg) = 0;
    };

    explicit UpgradeManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~UpgradeManager();

    UpgradeManager(const UpgradeManager&) = delete;
    UpgradeManager& operator=(const UpgradeManager&) = delete;

    /// Called once by Service::initService() — binds everything and creates FSM
    void initialize(Listener* listener);

    // ── Operations (called by Service, one-line delegation) ──

    void checkForUpgrade(bool userTriggered);
    void downloadUpgrade();
    void installAndRestart();
    void cancelDownload();
    void remindLater();

    // ── Queries ──

    [[nodiscard]] model::UpgradeState getUpgradeState() const;
    [[nodiscard]] std::optional<model::UpgradeInfo> getAvailableUpgrade() const;

private:
    // FSM context callback bindings
    void bindFsmCallbacks();

    // Notification forwarding to listener
    void notifyStateChanged(model::UpgradeState state);
    void notifyCheckCompleted(const model::UpgradeCheckResult& result);
    void notifyDownloadProgress(int64_t current, int64_t total);
    void notifyError(model::UpgradeErrorCode code, const std::string& msg);

    // Manager reset
    void resetAllManagers();
    void softResetManagers();

    // Helper: get current app info
    std::string getCurrentVersionString() const;
    std::string getCurrentPlatform() const;
    std::string getCurrentArch() const;

    // Auto-check timer
    void startAutoCheckTimer();
    void stopAutoCheckTimer();

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
    Listener* mListener{nullptr};

    // Sub-managers
    std::unique_ptr<UpgradeCheckManager>    mCheckManager;
    std::unique_ptr<UpgradeDownloadManager> mDownloadManager;
    std::unique_ptr<UpgradeInstallManager>  mInstallManager;

    // FSM
    upgrade::UpgradeContext mFsmContext{};
    std::unique_ptr<upgrade::UpgradeFSM> mFsm;

    // Auto-check timer
    std::jthread mAutoCheckThread;
};

} // namespace ucf::service
