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

inline void Extracting::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Extracting);
    ctx.triggerExtract(ctx.downloadedFilePath);
}

inline void ReadyToInstall::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::ReadyToInstall);
}

inline void Installing::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Installing);
    ctx.triggerInstall(ctx.stagingDir);
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
    ctx.triggerCancelDownload();
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
    -> fsm::TransitionTo<Extracting> { return {}; }

inline auto Verifying::onEvent(UpgradeContext& ctx, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    ctx.triggerHardReset();
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── Extracting ──

inline auto Extracting::onEvent(UpgradeContext& ctx, const EvExtractOk& e)
    -> fsm::TransitionTo<ReadyToInstall> {
    ctx.stagingDir = e.stagingDir;
    return {};
}

inline auto Extracting::onEvent(UpgradeContext& ctx, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    ctx.triggerHardReset();
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── ReadyToInstall ──

inline auto ReadyToInstall::onEvent(UpgradeContext&, const EvInstallStart&)
    -> fsm::TransitionTo<Installing> { return {}; }

// ── Installing ──

inline auto Installing::onEvent(UpgradeContext& ctx, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    ctx.triggerHardReset();
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
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
