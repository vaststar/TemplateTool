/// @file UpgradeTransitions.inl
/// @brief Inline implementations for Upgrade FSM state enter/event handlers.
/// @note  Included at the bottom of UpgradeStates.h — do NOT include directly.

#pragma once

namespace ucf::service::upgrade {

// ═══════════════════════════════════════════════════
// onEnter implementations
//   Pure side-effects: state notification + work kickoff.
//   Reset logic is owned by transition handlers (onEvent), because each path
//   into a state may need a different cleanup policy.
// ═══════════════════════════════════════════════════

inline void Idle::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Idle);
}

inline void Checking::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Checking);
    ctx.triggerCheckForUpgrade(userTriggered);
}

inline void UpgradeAvailable::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::UpgradeAvailable);
    // Note: onCheckCompleted is fired by Checking::onEvent(EvCheckSuccess)
    // so both "has upgrade" and "no upgrade" exits are notified from the same place.
}

inline void Downloading::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Downloading);
    ctx.triggerDownload(ctx.availableUpgrade->package.downloadUrl);
}

inline void Verifying::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Verifying);
    ctx.triggerVerify(ctx.downloadedFilePath);
}

inline void ReadyToInstall::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::ReadyToInstall);
}

inline void OpeningInstaller::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::OpeningInstaller);
    ctx.triggerOpenInstaller(ctx.downloadedFilePath);
}

inline void InstallerOpened::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::InstallerOpened);
}

inline void Failed::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Failed);
    ctx.onError(errorCode, errorMessage);
}

// ═══════════════════════════════════════════════════
// onEvent (transition) implementations
//   Reset policy lives in transitions:
//     • EvCancel  (Downloading → Idle):  softReset — keep partial file
//     • All other paths → Idle / Failed: hardReset — wipe caches & downloads
// ═══════════════════════════════════════════════════

// ── Idle ──

inline auto Idle::onEvent(UpgradeContext&, const EvCheckRequested& e)
    -> fsm::TransitionTo<Checking> {
    return fsm::TransitionTo<Checking>{Checking{.userTriggered = e.userTriggered}};
}

// ── Checking ──

inline auto Checking::onEvent(UpgradeContext& ctx, const EvCheckSuccess& e)
    -> fsm::TransitionTo<UpgradeAvailable> {
    ctx.availableUpgrade = e.info;
    ctx.onCheckCompleted(model::UpgradeCheckResult{true, e.info});
    return {};
}

inline auto Checking::onEvent(UpgradeContext& ctx, const EvCheckNoUpgrade&)
    -> fsm::TransitionTo<Idle> {
    ctx.onCheckCompleted(model::UpgradeCheckResult{false, {}});
    ctx.triggerHardReset();
    return {};
}

inline auto Checking::onEvent(UpgradeContext& ctx, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    ctx.triggerHardReset();
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── UpgradeAvailable ──

inline auto UpgradeAvailable::onEvent(UpgradeContext&, const EvCheckRequested& e)
    -> fsm::TransitionTo<Checking> {
    return fsm::TransitionTo<Checking>{Checking{.userTriggered = e.userTriggered}};
}

inline auto UpgradeAvailable::onEvent(UpgradeContext&, const EvDownloadStart&)
    -> fsm::TransitionTo<Downloading> { return {}; }

inline auto UpgradeAvailable::onEvent(UpgradeContext& ctx, const EvDismiss&)
    -> fsm::TransitionTo<Idle> {
    ctx.triggerHardReset();
    return {};
}

// ── Downloading ──

inline auto Downloading::onEvent(UpgradeContext& ctx, const EvProgress& e)
    -> fsm::Stay {
    ctx.onDownloadProgress(e.current, e.total);
    return {};
}

inline auto Downloading::onEvent(UpgradeContext& ctx, const EvDownloadDone& e)
    -> fsm::TransitionTo<Verifying> {
    ctx.downloadedFilePath = e.filePath;
    return {};
}

inline auto Downloading::onEvent(UpgradeContext& ctx, const EvCancel&)
    -> fsm::TransitionTo<Idle> {
    ctx.triggerSoftReset();   // preserve partial download for future resume
    return {};
}

inline auto Downloading::onEvent(UpgradeContext& ctx, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    ctx.triggerHardReset();
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── Verifying ──

inline auto Verifying::onEvent(UpgradeContext&, const EvVerifyOk&)
    -> fsm::TransitionTo<ReadyToInstall> { return {}; }

inline auto Verifying::onEvent(UpgradeContext& ctx, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    ctx.triggerHardReset();
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── ReadyToInstall ──

inline auto ReadyToInstall::onEvent(UpgradeContext&, const EvOpenInstaller&)
    -> fsm::TransitionTo<OpeningInstaller> { return {}; }

inline auto ReadyToInstall::onEvent(UpgradeContext& ctx, const EvDismiss&)
    -> fsm::TransitionTo<Idle> {
    // No open request has been accepted: the manager still owns this file.
    ctx.triggerHardReset();
    return {};
}

// ── OpeningInstaller ──

inline auto OpeningInstaller::onEvent(UpgradeContext& ctx, const EvInstallerOpenResult& e)
    -> fsm::OneOf<fsm::TransitionTo<InstallerOpened>, fsm::TransitionTo<ReadyToInstall>> {
    if (e.success && ctx.releaseInstaller(ctx.downloadedFilePath)) {
        return fsm::TransitionTo<InstallerOpened>{};
    }
    // Keep the verified file for retry. No hard reset may run on this path.
    ctx.onError(model::UpgradeErrorCode::OpenInstallerFailed,
        "Could not hand off the installer. Try again or open it manually: "
        + ctx.downloadedFilePath);
    return fsm::TransitionTo<ReadyToInstall>{};
}

// ── InstallerOpened ──

inline auto InstallerOpened::onEvent(UpgradeContext& ctx, const EvDismiss&)
    -> fsm::TransitionTo<Idle> {
    ctx.triggerHardReset(); // releaseInstaller already removed this file from cleanup.
    return {};
}

inline auto InstallerOpened::onEvent(UpgradeContext& ctx, const EvCheckRequested& e)
    -> fsm::TransitionTo<Checking> {
    ctx.triggerHardReset();
    return fsm::TransitionTo<Checking>{Checking{.userTriggered = e.userTriggered}};
}

// ── Failed ──

inline auto Failed::onEvent(UpgradeContext&, const EvCheckRequested& e)
    -> fsm::TransitionTo<Checking> {
    return fsm::TransitionTo<Checking>{Checking{.userTriggered = e.userTriggered}};
}

inline auto Failed::onEvent(UpgradeContext& ctx, const EvDismiss&)
    -> fsm::TransitionTo<Idle> {
    ctx.triggerHardReset();
    return {};
}

} // namespace ucf::service::upgrade
