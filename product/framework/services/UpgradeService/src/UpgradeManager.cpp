#include "UpgradeManager.h"
#include "UpgradeServiceLogger.h"
#include "check/UpgradeCheckManager.h"
#include "download/UpgradeDownloadManager.h"
#include "install/UpgradeInstallManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Utilities/OSUtils/OSUtils.h>

namespace ucf::service {

UpgradeManager::UpgradeManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
    , mCheckManager(std::make_unique<UpgradeCheckManager>(coreFramework))
    , mDownloadManager(std::make_unique<UpgradeDownloadManager>(coreFramework))
    , mInstallManager(std::make_unique<UpgradeInstallManager>(coreFramework))
{
    UPGRADE_LOG_DEBUG("UpgradeManager created");
}

UpgradeManager::~UpgradeManager()
{
    stopAutoCheckTimer();
    UPGRADE_LOG_DEBUG("UpgradeManager destroyed");
}

void UpgradeManager::initialize(Listener* listener)
{
    mListener = listener;

    // 1. Check for interrupted upgrade from last session
    mInstallManager->checkAndRecoverFromFailedUpgrade();

    // 2. Bind FSM context callbacks
    bindFsmCallbacks();

    // 3. Create FSM (starts in Idle)
    mFsm = std::make_unique<upgrade::UpgradeFSM>(mFsmContext);
    mFsm->setName("UpgradeFSM");

    mFsm->onTransition([](auto /*fromIdx*/, auto /*toIdx*/, auto from, auto to) {
        UPGRADE_LOG_INFO("FSM transition: " << from << " → " << to);
    });

    mFsm->onUnhandledEvent([](auto /*stateIdx*/, auto stateName, auto eventName) {
        UPGRADE_LOG_WARN("Unhandled event '" << eventName << "' in state '" << stateName << "'");
    });

    // 4. Start auto-check timer
    //startAutoCheckTimer();

    UPGRADE_LOG_INFO("UpgradeManager initialized");
}

void UpgradeManager::bindFsmCallbacks()
{
    // ── Notification callbacks → forwarded to Service via Listener ──

    mFsmContext.onStateChanged = [this](model::UpgradeState state) {
        notifyStateChanged(state);
    };

    mFsmContext.onCheckCompleted = [this](const model::UpgradeCheckResult& result) {
        notifyCheckCompleted(result);
    };

    mFsmContext.onDownloadProgress = [this](int64_t current, int64_t total) {
        notifyDownloadProgress(current, total);
    };

    mFsmContext.onError = [this](model::UpgradeErrorCode code, const std::string& msg) {
        notifyError(code, msg);
    };

    // ── Async operation triggers → delegate to sub-managers ──

    mFsmContext.triggerCheckForUpgrade = [this](bool userTriggered) {
        auto version  = getCurrentVersionString();
        auto platform = getCurrentPlatform();
        auto arch     = getCurrentArch();

        mCheckManager->checkForUpgrade(version, platform, arch, userTriggered,
            [this](bool success, const model::UpgradeCheckResult& result,
                   model::UpgradeErrorCode errCode, const std::string& errMsg) {
                if (!success) {
                    mFsm->processEvent(upgrade::EvError{errCode, errMsg});
                } else if (!result.hasUpgrade) {
                    mFsm->processEvent(upgrade::EvCheckNoUpgrade{});
                } else {
                    mFsm->processEvent(upgrade::EvCheckSuccess{result.upgradeInfo});
                }
            });
    };

    mFsmContext.triggerDownload = [this](const std::string& /*url*/) {
        auto& info = *mFsmContext.availableUpgrade;
        mDownloadManager->downloadPackage(info.package,
            // Progress callback
            [this](int64_t current, int64_t total) {
                mFsm->processEvent(upgrade::EvProgress{current, total});
            },
            // Completion callback
            [this](bool success, const std::string& path,
                   model::UpgradeErrorCode errCode, const std::string& errMsg) {
                if (success) {
                    mFsm->processEvent(upgrade::EvDownloadDone{path});
                } else {
                    mFsm->processEvent(upgrade::EvError{errCode, errMsg});
                }
            });
    };

    mFsmContext.triggerVerify = [this](const std::string& filePath) {
        auto& info = *mFsmContext.availableUpgrade;
        mDownloadManager->verifyPackage(filePath, info.package.sha256,
            [this](bool success, model::UpgradeErrorCode errCode, const std::string& errMsg) {
                if (success) {
                    mFsm->processEvent(upgrade::EvVerifyOk{});
                } else {
                    mFsm->processEvent(upgrade::EvError{errCode, errMsg});
                }
            });
    };

    mFsmContext.triggerExtract = [this](const std::string& packagePath) {
        mInstallManager->extractPackageToStaging(packagePath,
            [this](bool success, const std::string& stagingDir,
                   model::UpgradeErrorCode errCode, const std::string& errMsg) {
                if (success) {
                    mFsm->processEvent(upgrade::EvExtractOk{stagingDir});
                } else {
                    mFsm->processEvent(upgrade::EvError{errCode, errMsg});
                }
            });
    };

    mFsmContext.triggerInstall = [this](const std::string& stagingDir) {
        mInstallManager->launchUpdaterAndExit(stagingDir,
            [this](bool success, model::UpgradeErrorCode errCode, const std::string& errMsg) {
                if (!success) {
                    mFsm->processEvent(upgrade::EvError{errCode, errMsg});
                }
                // On success, the app is about to exit — no further events needed
            });
    };

    mFsmContext.triggerCancelDownload = [this]() {
        mDownloadManager->cancelDownload();
    };

    // ── Reset triggers ──

    mFsmContext.triggerHardReset = [this]() {
        hardResetManagers();
    };

    mFsmContext.triggerSoftReset = [this]() {
        softResetManagers();
    };
}

// ── Public operations (FSM event dispatch) ──

void UpgradeManager::checkForUpgrade(bool userTriggered)
{
    mFsm->processEvent(upgrade::EvCheckRequested{userTriggered});
}

void UpgradeManager::downloadUpgrade()
{
    mFsm->processEvent(upgrade::EvDownloadStart{});
}

void UpgradeManager::installAndRestart()
{
    mFsm->processEvent(upgrade::EvInstallStart{});
}

void UpgradeManager::cancelDownload()
{
    mFsm->processEvent(upgrade::EvCancel{});
}

void UpgradeManager::dismissUpgrade()
{
    mFsm->processEvent(upgrade::EvDismiss{});
}

// ── Queries ──

model::UpgradeState UpgradeManager::getUpgradeState() const
{
    if (!mFsm) {
        return model::UpgradeState::Idle;
    }
    return mFsm->visitState([](const auto& state) -> model::UpgradeState {
        using S = std::decay_t<decltype(state)>;
        if constexpr (std::is_same_v<S, upgrade::Idle>)            return model::UpgradeState::Idle;
        if constexpr (std::is_same_v<S, upgrade::Checking>)        return model::UpgradeState::Checking;
        if constexpr (std::is_same_v<S, upgrade::UpgradeAvailable>) return model::UpgradeState::UpgradeAvailable;
        if constexpr (std::is_same_v<S, upgrade::Downloading>)     return model::UpgradeState::Downloading;
        if constexpr (std::is_same_v<S, upgrade::Verifying>)       return model::UpgradeState::Verifying;
        if constexpr (std::is_same_v<S, upgrade::Extracting>)      return model::UpgradeState::Extracting;
        if constexpr (std::is_same_v<S, upgrade::ReadyToInstall>)  return model::UpgradeState::ReadyToInstall;
        if constexpr (std::is_same_v<S, upgrade::Installing>)      return model::UpgradeState::Installing;
        if constexpr (std::is_same_v<S, upgrade::Failed>)          return model::UpgradeState::Failed;
    });
}

std::optional<model::UpgradeInfo> UpgradeManager::getAvailableUpgrade() const
{
    return mFsmContext.availableUpgrade;
}

// ── Notification forwarding ──

void UpgradeManager::notifyStateChanged(model::UpgradeState state)
{
    if (mListener) {
        mListener->onUpgradeStateChanged(state);
    }
}

void UpgradeManager::notifyCheckCompleted(const model::UpgradeCheckResult& result)
{
    if (mListener) {
        mListener->onUpgradeCheckCompleted(result);
    }
}

void UpgradeManager::notifyDownloadProgress(int64_t current, int64_t total)
{
    if (mListener) {
        mListener->onDownloadProgressChanged(current, total);
    }
}

void UpgradeManager::notifyError(model::UpgradeErrorCode code, const std::string& msg)
{
    if (mListener) {
        mListener->onUpgradeError(code, msg);
    }
}

// ── Manager reset ──

void UpgradeManager::hardResetManagers()
{
    UPGRADE_LOG_DEBUG("Hard reset all managers (clear caches and partial downloads)");
    mCheckManager->reset();
    mDownloadManager->hardReset();
    mInstallManager->reset();
    mFsmContext.availableUpgrade.reset();
    mFsmContext.downloadedFilePath.clear();
    mFsmContext.stagingDir.clear();
}

void UpgradeManager::softResetManagers()
{
    UPGRADE_LOG_DEBUG("Soft reset managers (preserve partial download for resume)");
    mCheckManager->reset();
    mDownloadManager->softReset();
    mInstallManager->reset();
    // Keep ctx.availableUpgrade so a subsequent EvDownloadStart can resume.
    // downloadedFilePath / stagingDir remain empty until that download progresses.
}

// ── Helper: app info ──

std::string UpgradeManager::getCurrentVersionString() const
{
    auto coreFramework = mCoreFramework.lock();
    if (coreFramework) {
        if (auto clientInfo = coreFramework->getService<IClientInfoService>().lock()) {
            return clientInfo->getApplicationVersion().toString();
        }
    }
    return "0.0.0.0";
}

std::string UpgradeManager::getCurrentPlatform() const
{
    auto name = ucf::utilities::OSUtils::getOSTypeName();
    // Manifest uses lowercase: "windows", "linux", "macos"
    if (name == "Windows") { return "windows"; }
    if (name == "Linux")   { return "linux"; }
    if (name == "macOS")   { return "macos"; }
    return name;
}

std::string UpgradeManager::getCurrentArch() const
{
    auto arch = ucf::utilities::OSUtils::getCPUArch();
    // Manifest uses "x64" instead of "x86_64"
    if (arch == "x86_64") { return "x64"; }
    return arch;
}

// ── Auto-check timer ──

void UpgradeManager::startAutoCheckTimer()
{
    mStopRequested.store(false);
    mAutoCheckThread = std::thread([this]() {
        // Initial delay: 30 seconds after startup
        for (int i = 0; i < 30 && !mStopRequested.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        while (!mStopRequested.load()) {
            UPGRADE_LOG_DEBUG("Auto-check timer triggered");
            checkForUpgrade(/*userTriggered=*/false);

            // Check every 4 hours
            for (int i = 0; i < 4 * 3600 && !mStopRequested.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    });
}

void UpgradeManager::stopAutoCheckTimer()
{
    mStopRequested.store(true);
    if (mAutoCheckThread.joinable()) {
        mAutoCheckThread.join();
    }
}

} // namespace ucf::service
