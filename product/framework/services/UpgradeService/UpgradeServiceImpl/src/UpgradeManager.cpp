#include "UpgradeManager.h"
#include "UpgradeServiceLogger.h"
#include "check/UpgradeCheckManager.h"
#include "download/UpgradeDownloadManager.h"
#include "install/UpgradeInstallManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>
#include <ucf/utilities/OSUtils/OSUtils.h>

#include <chrono>
#include <functional>
#include <type_traits>
#include <utility>

namespace ucf::service {

namespace {

// The captured gate remains alive after UpgradeManager is destroyed. A callback
// may use its captured `this` only while it holds a lease.
template <typename Callback>
auto withCallbackGate(std::shared_ptr<AsyncCallbackGate> gate, Callback callback)
{
    return [gate = std::move(gate), callback = std::move(callback)](auto&&... args) mutable {
        auto lease = gate->tryEnter();
        if (!lease) {
            return;
        }
        std::invoke(callback, std::forward<decltype(args)>(args)...);
    };
}

} // namespace

UpgradeManager::UpgradeManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
    , mCheckManager(std::make_unique<UpgradeCheckManager>(coreFramework, mCallbackGate))
    , mDownloadManager(std::make_unique<UpgradeDownloadManager>(coreFramework, mCallbackGate))
    , mInstallManager(std::make_unique<UpgradeInstallManager>(coreFramework))
{
    UPGRADE_LOG_DEBUG("UpgradeManager constructing, address: " << this);
    UPGRADE_LOG_DEBUG("UpgradeManager constructed, address: " << this);
}

UpgradeManager::~UpgradeManager()
{
    UPGRADE_LOG_DEBUG("UpgradeManager destroying, address: " << this);
    shutdown();
    UPGRADE_LOG_DEBUG("UpgradeManager destructor body finished, address: " << this);
}

void UpgradeManager::shutdown()
{
    std::call_once(mShutdownOnce, [this] {
        // Reject new external calls and callbacks before touching the workers.
        mCallbackGate->close();
        mStopRequested.store(true, std::memory_order_release);
        mDownloadManager->requestStop();
        stopAutoCheckTimer();

        // cancelRequest synchronously invokes completion callbacks; no FSM,
        // session, or callback-gate lock may be held here.
        mCheckManager->cancelOutstandingRequests();
        mDownloadManager->cancelOutstandingRequests();

        mDownloadManager->joinWorkers();
        mCallbackGate->waitForDrain();

        // An already admitted send can submit a request after the first sweep.
        mCheckManager->cancelOutstandingRequests();
        mDownloadManager->cancelOutstandingRequests();
        // A callback admitted before close() may enqueue cleanup while the
        // first join runs. Drain those file jobs after all leases have left.
        mDownloadManager->joinWorkers();
        mListener = nullptr;
    });
}

void UpgradeManager::initialize(Listener* listener)
{
    UPGRADE_LOG_INFO("UpgradeManager initialization started, address: " << this);

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

    UPGRADE_LOG_INFO("UpgradeManager initialization finished, address: " << this);
}

void UpgradeManager::bindFsmCallbacks()
{
    // ── Notification callbacks → forwarded to Service via Listener ──

    mFsmContext.onStateChanged = withCallbackGate(mCallbackGate, [this](model::UpgradeState state) {
        notifyStateChanged(state);
    });

    mFsmContext.onCheckCompleted = withCallbackGate(mCallbackGate, [this](const model::UpgradeCheckResult& result) {
        notifyCheckCompleted(result);
    });

    mFsmContext.onDownloadProgress = withCallbackGate(mCallbackGate, [this](int64_t current, int64_t total) {
        notifyDownloadProgress(current, total);
    });

    mFsmContext.onError = withCallbackGate(mCallbackGate, [this](model::UpgradeErrorCode code, const std::string& msg) {
        notifyError(code, msg);
    });

    // ── Async operation triggers → delegate to sub-managers ──

    mFsmContext.triggerCheckForUpgrade = withCallbackGate(mCallbackGate, [this](bool userTriggered) {
        auto version  = getCurrentVersionString();
        auto platform = getCurrentPlatform();
        auto arch     = getCurrentArch();

        mCheckManager->checkForUpgrade(version, platform, arch, userTriggered,
            withCallbackGate(mCallbackGate, [this](bool success, const model::UpgradeCheckResult& result,
                   model::UpgradeErrorCode errCode, const std::string& errMsg) {
                if (!success) {
                    mFsm->processEvent(upgrade::EvError{errCode, errMsg});
                } else if (!result.hasUpgrade) {
                    mFsm->processEvent(upgrade::EvCheckNoUpgrade{});
                } else {
                    mFsm->processEvent(upgrade::EvCheckSuccess{result.upgradeInfo});
                }
            }));
    });

    mFsmContext.triggerDownload = withCallbackGate(mCallbackGate, [this](const std::string& /*url*/) {
        if (!mFsmContext.availableUpgrade) {
            mFsm->processEvent(upgrade::EvError{
                model::UpgradeErrorCode::DownloadFailed, "No available upgrade to download"});
            return;
        }
        auto& info = *mFsmContext.availableUpgrade;
        mDownloadManager->downloadPackage(info.package,
            // Progress callback
            withCallbackGate(mCallbackGate, [this](int64_t current, int64_t total) {
                mFsm->processEvent(upgrade::EvProgress{current, total});
            }),
            // Completion callback
            withCallbackGate(mCallbackGate, [this](bool success, const std::string& path,
                   model::UpgradeErrorCode errCode, const std::string& errMsg) {
                if (success) {
                    mFsm->processEvent(upgrade::EvDownloadDone{path});
                } else {
                    mFsm->processEvent(upgrade::EvError{errCode, errMsg});
                }
            }));
    });

    mFsmContext.triggerVerify = withCallbackGate(mCallbackGate, [this](const std::string& filePath) {
        if (!mFsmContext.availableUpgrade) {
            mFsm->processEvent(upgrade::EvError{
                model::UpgradeErrorCode::VerifyFailed, "No available upgrade to verify"});
            return;
        }
        auto& info = *mFsmContext.availableUpgrade;
        mDownloadManager->verifyPackage(filePath, info.package.sha256,
            withCallbackGate(mCallbackGate, [this](bool success, model::UpgradeErrorCode errCode, const std::string& errMsg) {
                if (success) {
                    mFsm->processEvent(upgrade::EvVerifyOk{});
                } else {
                    mFsm->processEvent(upgrade::EvError{errCode, errMsg});
                }
            }));
    });

    mFsmContext.triggerExtract = withCallbackGate(mCallbackGate, [this](const std::string& packagePath) {
        mInstallManager->extractPackageToStaging(packagePath,
            withCallbackGate(mCallbackGate, [this](bool success, const std::string& stagingDir,
                   model::UpgradeErrorCode errCode, const std::string& errMsg) {
                if (success) {
                    mFsm->processEvent(upgrade::EvExtractOk{stagingDir});
                } else {
                    mFsm->processEvent(upgrade::EvError{errCode, errMsg});
                }
            }));
    });

    mFsmContext.triggerInstall = withCallbackGate(mCallbackGate, [this](const std::string& stagingDir) {
        mInstallManager->launchUpdaterAndExit(stagingDir,
            withCallbackGate(mCallbackGate, [this](bool success, model::UpgradeErrorCode errCode, const std::string& errMsg) {
                if (!success) {
                    mFsm->processEvent(upgrade::EvError{errCode, errMsg});
                }
                // On success, the app is about to exit — no further events needed
            }));
    });

    // ── Reset triggers ──

    mFsmContext.triggerHardReset = withCallbackGate(mCallbackGate, [this]() {
        hardResetManagers();
    });

    mFsmContext.triggerSoftReset = withCallbackGate(mCallbackGate, [this]() {
        softResetManagers();
    });
}

// ── Public operations (FSM event dispatch) ──

void UpgradeManager::checkForUpgrade(bool userTriggered)
{
    auto lease = mCallbackGate->tryEnter();
    if (!lease) { return; }
    mFsm->processEvent(upgrade::EvCheckRequested{userTriggered});
}

void UpgradeManager::downloadUpgrade()
{
    auto lease = mCallbackGate->tryEnter();
    if (!lease) { return; }
    mFsm->processEvent(upgrade::EvDownloadStart{});
}

void UpgradeManager::installAndRestart()
{
    auto lease = mCallbackGate->tryEnter();
    if (!lease) { return; }
    mFsm->processEvent(upgrade::EvInstallStart{});
}

void UpgradeManager::cancelDownload()
{
    auto lease = mCallbackGate->tryEnter();
    if (!lease) { return; }
    mFsm->processEvent(upgrade::EvCancel{});
}

void UpgradeManager::dismissUpgrade()
{
    auto lease = mCallbackGate->tryEnter();
    if (!lease) { return; }
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
    if (!mFsm) {
        return std::nullopt;
    }
    return mFsm->withContext([](const upgrade::UpgradeContext& context) {
        return context.availableUpgrade;
    });
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
    UPGRADE_LOG_INFO("UpgradeManager auto-check timer startup started, address: " << this);

    mStopRequested.store(false);
    mAutoCheckThread = std::thread([this]() {
        UPGRADE_LOG_DEBUG("UpgradeManager auto-check thread started, address: " << this);

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

        UPGRADE_LOG_DEBUG("UpgradeManager auto-check thread finished, address: " << this);
    });

    UPGRADE_LOG_INFO("UpgradeManager auto-check timer startup finished, address: " << this);
}

void UpgradeManager::stopAutoCheckTimer()
{
    UPGRADE_LOG_INFO("UpgradeManager auto-check timer shutdown started, address: " << this);

    mStopRequested.store(true);
    if (mAutoCheckThread.joinable()) {
        mAutoCheckThread.join();
    }

    UPGRADE_LOG_INFO("UpgradeManager auto-check timer shutdown finished, address: " << this);
}

} // namespace ucf::service
