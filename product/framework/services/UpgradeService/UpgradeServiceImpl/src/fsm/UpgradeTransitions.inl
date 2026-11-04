/// @file UpgradeTransitions.inl
/// @brief Inline implementations for Upgrade FSM state enter/event handlers.
/// @note  Included at the bottom of UpgradeStates.h — do NOT include directly.

#pragma once

namespace ucf::service::upgrade {

// ═══════════════════════════════════════════════════
// onEnter / onExit implementations
//   Pure side-effects: state notification + work kickoff.
//   Async states own one operation from entry through exit.
//   Reset logic is owned by transition handlers (onEvent), because each path
//   into a state may need a different cleanup policy.
// ═══════════════════════════════════════════════════

inline void Idle::onEnter(UpgradeContext& ctx)
{
    ctx.onStateChanged(model::UpgradeState::Idle);
}

inline void Checking::onEnter(UpgradeContext& ctx)
{
    const auto operation = ctx.beginOperation();
    ctx.onStateChanged(model::UpgradeState::Checking);
    // The check includes the asynchronous local cache lookup after the manifest.
    ctx.triggerCheckForUpgrade(operation, userTriggered);
}

inline void Checking::onExit(UpgradeContext& ctx)
{
    ctx.endOperation();
}

inline void UpgradeAvailable::onEnter(UpgradeContext& ctx)
{
    ctx.onStateChanged(model::UpgradeState::UpgradeAvailable);
    // Note: onCheckCompleted is fired by Checking::onEvent(EvCheckSuccess)
    // so both "has upgrade" and "no upgrade" exits are notified from the same place.
}

inline void Downloading::onEnter(UpgradeContext& ctx)
{
    const auto operation = ctx.beginOperation();
    ctx.onStateChanged(model::UpgradeState::Downloading);
    ctx.triggerDownload(operation, ctx.availableUpgrade->package);
}

inline void Downloading::onExit(UpgradeContext& ctx)
{
    ctx.endOperation();
}

inline void Verifying::onEnter(UpgradeContext& ctx)
{
    const auto operation = ctx.beginOperation();
    ctx.onStateChanged(model::UpgradeState::Verifying);
    ctx.triggerVerify(operation, ctx.downloadedFilePath, ctx.availableUpgrade->package);
}

inline void Verifying::onExit(UpgradeContext& ctx)
{
    ctx.endOperation();
}

inline void ReadyToInstall::onEnter(UpgradeContext& ctx)
{
    ctx.onStateChanged(model::UpgradeState::ReadyToInstall);
}

inline void OpeningInstaller::onEnter(UpgradeContext& ctx)
{
    ctx.onStateChanged(model::UpgradeState::OpeningInstaller);
    ctx.triggerOpenInstaller(ctx.downloadedFilePath);
}

inline void InstallerOpened::onEnter(UpgradeContext& ctx)
{
    ctx.onStateChanged(model::UpgradeState::InstallerOpened);
}

inline void Failed::onEnter(UpgradeContext& ctx)
{
    ctx.onStateChanged(model::UpgradeState::Failed);
    ctx.onError(errorCode, errorMessage);
}

// ═══════════════════════════════════════════════════
// onEvent (transition) implementations
//   Reset policy lives in transitions:
//     • EvCancel  (Downloading → Idle):  softReset — keep partial file
//     • Cached verification failure → UpgradeAvailable: keep the manifest so the
//       user can download; the download manager discards the invalid cache.
//     • All other paths → Idle / Failed: hardReset — clear incomplete downloads,
//       retaining the last verified installer for reuse, including after restart.
// ═══════════════════════════════════════════════════

// ── Idle ──

inline auto Idle::onEvent(UpgradeContext&, const EvCheckRequested& e) -> fsm::TransitionTo<Checking>
{
    return fsm::TransitionTo<Checking>{Checking{.userTriggered = e.userTriggered}};
}

// ── Checking ──

inline auto Checking::onEvent(UpgradeContext& ctx, const EvCheckSuccess& e)
    -> fsm::OneOf<fsm::TransitionTo<Verifying>, fsm::TransitionTo<UpgradeAvailable>, fsm::Stay>
{
    if (!ctx.isCurrent(e.operationId))
    {
        return fsm::Stay{};
    }
    ctx.availableUpgrade = e.info;
    ctx.downloadedFilePath = e.cachedFilePath.value_or("");
    ctx.onCheckCompleted(model::UpgradeCheckResult{true, e.info});
    if (!ctx.downloadedFilePath.empty())
    {
        return fsm::TransitionTo<Verifying>{Verifying{.fromCache = true}};
    }
    return fsm::TransitionTo<UpgradeAvailable>{};
}

inline auto Checking::onEvent(UpgradeContext& ctx, const EvCheckNoUpgrade& e)
    -> fsm::OneOf<fsm::TransitionTo<Idle>, fsm::Stay>
{
    if (!ctx.isCurrent(e.operationId))
    {
        return fsm::Stay{};
    }
    ctx.onCheckCompleted(model::UpgradeCheckResult{false, {}});
    ctx.hardReset();
    return fsm::TransitionTo<Idle>{};
}

inline auto Checking::onEvent(UpgradeContext& ctx, const EvError& e) -> fsm::OneOf<fsm::TransitionTo<Failed>, fsm::Stay>
{
    if (!ctx.isCurrent(e.operationId))
    {
        return fsm::Stay{};
    }
    ctx.hardReset();
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── UpgradeAvailable ──

inline auto UpgradeAvailable::onEvent(UpgradeContext&, const EvCheckRequested& e) -> fsm::TransitionTo<Checking>
{
    return fsm::TransitionTo<Checking>{Checking{.userTriggered = e.userTriggered}};
}

inline auto UpgradeAvailable::onEvent(UpgradeContext& ctx, const EvDownloadStart&)
    -> fsm::OneOf<fsm::TransitionTo<Downloading>, fsm::TransitionTo<Failed>>
{
    if (!ctx.availableUpgrade)
    {
        ctx.hardReset();
        return fsm::TransitionTo<Failed>{
            Failed{.errorCode = model::UpgradeErrorCode::DownloadFailed,
                .errorMessage = "Upgrade information is unavailable"}};
    }
    return fsm::TransitionTo<Downloading>{};
}

inline auto UpgradeAvailable::onEvent(UpgradeContext& ctx, const EvDismiss&) -> fsm::TransitionTo<Idle>
{
    ctx.hardReset();
    return {};
}

// ── Downloading ──

inline auto Downloading::onEvent(UpgradeContext& ctx, const EvProgress& e) -> fsm::Stay
{
    if (!ctx.isCurrent(e.operationId))
    {
        return fsm::Stay{};
    }
    ctx.onDownloadProgress(e.current, e.total);
    return fsm::Stay{};
}

inline auto Downloading::onEvent(UpgradeContext& ctx, const EvDownloadDone& e)
    -> fsm::OneOf<fsm::TransitionTo<Verifying>, fsm::TransitionTo<Failed>, fsm::Stay>
{
    if (!ctx.isCurrent(e.operationId))
    {
        return fsm::Stay{};
    }
    if (!ctx.availableUpgrade || e.filePath.empty())
    {
        ctx.hardReset();
        return fsm::TransitionTo<Failed>{
            Failed{.errorCode = model::UpgradeErrorCode::VerifyFailed,
                .errorMessage = "Completed installer information is unavailable"}};
    }
    ctx.downloadedFilePath = e.filePath;
    return fsm::TransitionTo<Verifying>{};
}

inline auto Downloading::onEvent(UpgradeContext& ctx, const EvCancel&) -> fsm::TransitionTo<Idle>
{
    ctx.softReset(); // preserve partial download for future resume
    return {};
}

inline auto Downloading::onEvent(UpgradeContext& ctx, const EvError& e)
    -> fsm::OneOf<fsm::TransitionTo<Failed>, fsm::Stay>
{
    if (!ctx.isCurrent(e.operationId))
    {
        return fsm::Stay{};
    }
    ctx.hardReset();
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── Verifying ──

inline auto Verifying::onEvent(UpgradeContext& ctx, const EvVerifyOk& e)
    -> fsm::OneOf<fsm::TransitionTo<ReadyToInstall>, fsm::Stay>
{
    if (!ctx.isCurrent(e.operationId))
    {
        return fsm::Stay{};
    }
    return fsm::TransitionTo<ReadyToInstall>{};
}

inline auto Verifying::onEvent(UpgradeContext& ctx, const EvError& e)
    -> fsm::OneOf<fsm::TransitionTo<UpgradeAvailable>, fsm::TransitionTo<Failed>, fsm::Stay>
{
    if (!ctx.isCurrent(e.operationId))
    {
        return fsm::Stay{};
    }
    if (fromCache)
    {
        // A stale local candidate is a cache miss, not a failed new download.
        // Keep availableUpgrade; the download manager owns invalid-cache cleanup.
        ctx.downloadedFilePath.clear();
        return fsm::TransitionTo<UpgradeAvailable>{};
    }
    ctx.hardReset();
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

// ── ReadyToInstall ──

inline auto ReadyToInstall::onEvent(UpgradeContext&, const EvOpenInstaller&) -> fsm::TransitionTo<OpeningInstaller>
{
    return {};
}

inline auto ReadyToInstall::onEvent(UpgradeContext& ctx, const EvDismiss&) -> fsm::TransitionTo<Idle>
{
    // Keep the verified installer cached for a later check, including after restart.
    ctx.hardReset();
    return {};
}

// ── OpeningInstaller ──

inline auto OpeningInstaller::onEvent(UpgradeContext& ctx, const EvInstallerOpenResult& e)
    -> fsm::OneOf<fsm::TransitionTo<InstallerOpened>, fsm::TransitionTo<ReadyToInstall>>
{
    if (e.success && ctx.releaseInstaller(ctx.downloadedFilePath))
    {
        return fsm::TransitionTo<InstallerOpened>{};
    }
    // Keep the verified file for retry. No hard reset may run on this path.
    ctx.onError(model::UpgradeErrorCode::OpenInstallerFailed,
        "Could not hand off the installer. Try again or open it manually: " + ctx.downloadedFilePath);
    return fsm::TransitionTo<ReadyToInstall>{};
}

// ── InstallerOpened ──

inline auto InstallerOpened::onEvent(UpgradeContext& ctx, const EvDismiss&) -> fsm::TransitionTo<Idle>
{
    ctx.hardReset(); // releaseInstaller already removed this file from cleanup.
    return {};
}

inline auto InstallerOpened::onEvent(UpgradeContext& ctx, const EvCheckRequested& e) -> fsm::TransitionTo<Checking>
{
    ctx.hardReset();
    return fsm::TransitionTo<Checking>{Checking{.userTriggered = e.userTriggered}};
}

// ── Failed ──

inline auto Failed::onEvent(UpgradeContext&, const EvCheckRequested& e) -> fsm::TransitionTo<Checking>
{
    return fsm::TransitionTo<Checking>{Checking{.userTriggered = e.userTriggered}};
}

inline auto Failed::onEvent(UpgradeContext& ctx, const EvDismiss&) -> fsm::TransitionTo<Idle>
{
    ctx.hardReset();
    return {};
}

} // namespace ucf::service::upgrade
