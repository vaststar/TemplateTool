/// @file UpgradeTransitions.inl
/// @brief Inline implementations for Upgrade FSM state enter/event handlers.
/// @note  Included at the bottom of UpgradeStates.h — do NOT include directly.

#pragma once

namespace ucf::service::upgrade {

// ═══════════════════════════════════════════════════
// onEnter implementations
// ═══════════════════════════════════════════════════

inline void Idle::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Idle);
    if (ctx.keepPartialDownload) {
        ctx.keepPartialDownload = false;
        if (ctx.triggerSoftResetManagers) {
            ctx.triggerSoftResetManagers();
        }
    } else {
        if (ctx.triggerResetManagers) {
            ctx.triggerResetManagers();
        }
    }
}

inline void Checking::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Checking);
    ctx.triggerCheckForUpgrade(userTriggered);
}

inline void UpgradeAvailable::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::UpgradeAvailable);
    ctx.onCheckCompleted(model::UpgradeCheckResult{true, *ctx.availableUpgrade});
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

inline void Installing::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Installing);
    ctx.triggerInstall(ctx.downloadedFilePath);
}

inline void Failed::onEnter(UpgradeContext& ctx) {
    ctx.onStateChanged(model::UpgradeState::Failed);
    ctx.onError(errorCode, errorMessage);
}

// ═══════════════════════════════════════════════════
// onEvent (transition) implementations
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
    return {};
}

inline auto Checking::onEvent(UpgradeContext& ctx, const EvCheckNoUpgrade&)
    -> fsm::TransitionTo<Idle> {
    ctx.onCheckCompleted(model::UpgradeCheckResult{false, {}});
    return {};
}

inline auto Checking::onEvent(UpgradeContext&, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── UpgradeAvailable ──

inline auto UpgradeAvailable::onEvent(UpgradeContext&, const EvDownloadStart&)
    -> fsm::TransitionTo<Downloading> { return {}; }

inline auto UpgradeAvailable::onEvent(UpgradeContext&, const EvRemindLater&)
    -> fsm::TransitionTo<Idle> { return {}; }

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
    ctx.keepPartialDownload = true;
    return {};
}

inline auto Downloading::onEvent(UpgradeContext&, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── Verifying ──

inline auto Verifying::onEvent(UpgradeContext&, const EvVerifyOk&)
    -> fsm::TransitionTo<ReadyToInstall> { return {}; }

inline auto Verifying::onEvent(UpgradeContext&, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── ReadyToInstall ──

inline auto ReadyToInstall::onEvent(UpgradeContext&, const EvInstallStart&)
    -> fsm::TransitionTo<Installing> { return {}; }

// ── Installing ──

inline auto Installing::onEvent(UpgradeContext&, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── Failed ──

inline auto Failed::onEvent(UpgradeContext&, const EvReset&)
    -> fsm::TransitionTo<Idle> { return {}; }

} // namespace ucf::service::upgrade
