#pragma once

/// @file UpgradeStates.h
/// @brief State types, context, and FSM typedef for the Upgrade state machine.

#include "UpgradeEvents.h"
#include <ucf/Utilities/StateMachineUtils/StateMachine.h>
#include <functional>
#include <optional>
#include <string>

namespace fsm = ucf::utilities::fsm;

namespace ucf::service::upgrade {

// ── Forward declare all states ──
struct Idle;
struct Checking;
struct UpgradeAvailable;
struct Downloading;
struct Verifying;
struct ReadyToInstall;
struct Installing;
struct Failed;

// ═══════════════════════════════════════
// Context — shared data borrowed by FSM
// ═══════════════════════════════════════
struct UpgradeContext {
    // ── Notification callbacks (Service → UI) ──
    std::function<void(model::UpgradeState)>                              onStateChanged;
    std::function<void(const model::UpgradeCheckResult&)>                 onCheckCompleted;
    std::function<void(int64_t, int64_t)>                                 onDownloadProgress;
    std::function<void(model::UpgradeErrorCode, const std::string&)>      onError;

    // ── Async operation triggers (bound to Manager methods) ──
    std::function<void(bool userTriggered)>      triggerCheckForUpgrade;
    std::function<void(const std::string& url)>  triggerDownload;
    std::function<void(const std::string& path)> triggerVerify;
    std::function<void(const std::string& path)> triggerInstall;
    std::function<void()>                        triggerCancelDownload;

    // ── Reset triggers ──
    std::function<void()>                        triggerResetManagers;
    std::function<void()>                        triggerSoftResetManagers;

    // ── Shared data ──
    std::optional<model::UpgradeInfo> availableUpgrade;
    std::string downloadedFilePath;
    bool keepPartialDownload{false};
};

// ═══════════════════════════════════════
// State definitions (declarations only)
// ═══════════════════════════════════════

struct Idle {
    static constexpr std::string_view name() { return "Idle"; }

    void onEnter(UpgradeContext& ctx) {
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

    auto onEvent(UpgradeContext& ctx, const EvCheckRequested& e)
        -> fsm::TransitionTo<Checking>;
};

struct Checking {
    static constexpr std::string_view name() { return "Checking"; }
    bool userTriggered{false};

    void onEnter(UpgradeContext& ctx) {
        ctx.onStateChanged(model::UpgradeState::Checking);
        ctx.triggerCheckForUpgrade(userTriggered);
    }

    auto onEvent(UpgradeContext& ctx, const EvCheckSuccess& e)
        -> fsm::TransitionTo<UpgradeAvailable>;
    auto onEvent(UpgradeContext& ctx, const EvCheckNoUpgrade&)
        -> fsm::TransitionTo<Idle>;
    auto onEvent(UpgradeContext& ctx, const EvError& e)
        -> fsm::TransitionTo<Failed>;
};

struct UpgradeAvailable {
    static constexpr std::string_view name() { return "UpgradeAvailable"; }

    void onEnter(UpgradeContext& ctx) {
        ctx.onStateChanged(model::UpgradeState::UpgradeAvailable);
        ctx.onCheckCompleted(model::UpgradeCheckResult{true, *ctx.availableUpgrade});
    }

    auto onEvent(UpgradeContext& ctx, const EvDownloadStart&)
        -> fsm::TransitionTo<Downloading>;
    auto onEvent(UpgradeContext& ctx, const EvRemindLater&)
        -> fsm::TransitionTo<Idle>;
};

struct Downloading {
    static constexpr std::string_view name() { return "Downloading"; }

    void onEnter(UpgradeContext& ctx) {
        ctx.onStateChanged(model::UpgradeState::Downloading);
        ctx.triggerDownload(ctx.availableUpgrade->package.downloadUrl);
    }

    auto onEvent(UpgradeContext& ctx, const EvProgress& e) -> fsm::Stay {
        ctx.onDownloadProgress(e.current, e.total);
        return {};
    }

    auto onEvent(UpgradeContext& ctx, const EvDownloadDone& e)
        -> fsm::TransitionTo<Verifying>;
    auto onEvent(UpgradeContext& ctx, const EvCancel&)
        -> fsm::TransitionTo<Idle>;
    auto onEvent(UpgradeContext& ctx, const EvError& e)
        -> fsm::TransitionTo<Failed>;
};

struct Verifying {
    static constexpr std::string_view name() { return "Verifying"; }

    void onEnter(UpgradeContext& ctx) {
        ctx.onStateChanged(model::UpgradeState::Verifying);
        ctx.triggerVerify(ctx.downloadedFilePath);
    }

    auto onEvent(UpgradeContext& ctx, const EvVerifyOk&)
        -> fsm::TransitionTo<ReadyToInstall>;
    auto onEvent(UpgradeContext& ctx, const EvError& e)
        -> fsm::TransitionTo<Failed>;
};

struct ReadyToInstall {
    static constexpr std::string_view name() { return "ReadyToInstall"; }

    void onEnter(UpgradeContext& ctx) {
        ctx.onStateChanged(model::UpgradeState::ReadyToInstall);
    }

    auto onEvent(UpgradeContext& ctx, const EvInstallStart&)
        -> fsm::TransitionTo<Installing>;
};

struct Installing {
    static constexpr std::string_view name() { return "Installing"; }

    void onEnter(UpgradeContext& ctx) {
        ctx.onStateChanged(model::UpgradeState::Installing);
        ctx.triggerInstall(ctx.downloadedFilePath);
    }

    auto onEvent(UpgradeContext& ctx, const EvError& e)
        -> fsm::TransitionTo<Failed>;
};

struct Failed {
    static constexpr std::string_view name() { return "Failed"; }
    model::UpgradeErrorCode errorCode{};
    std::string errorMessage;

    void onEnter(UpgradeContext& ctx) {
        ctx.onStateChanged(model::UpgradeState::Failed);
        ctx.onError(errorCode, errorMessage);
    }

    auto onEvent(UpgradeContext& ctx, const EvReset&)
        -> fsm::TransitionTo<Idle>;
};

// ═══════════════════════════════════════
// Out-of-line transition definitions
// (all state types must be complete)
// ═══════════════════════════════════════

inline auto Idle::onEvent(UpgradeContext&, const EvCheckRequested& e)
    -> fsm::TransitionTo<Checking> {
    return fsm::TransitionTo<Checking>{Checking{.userTriggered = e.userTriggered}};
}

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

inline auto UpgradeAvailable::onEvent(UpgradeContext&, const EvDownloadStart&)
    -> fsm::TransitionTo<Downloading> { return {}; }

inline auto UpgradeAvailable::onEvent(UpgradeContext&, const EvRemindLater&)
    -> fsm::TransitionTo<Idle> { return {}; }

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

inline auto Verifying::onEvent(UpgradeContext&, const EvVerifyOk&)
    -> fsm::TransitionTo<ReadyToInstall> { return {}; }

inline auto Verifying::onEvent(UpgradeContext&, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

inline auto ReadyToInstall::onEvent(UpgradeContext&, const EvInstallStart&)
    -> fsm::TransitionTo<Installing> { return {}; }

inline auto Installing::onEvent(UpgradeContext&, const EvError& e)
    -> fsm::TransitionTo<Failed> {
    return fsm::TransitionTo<Failed>{Failed{.errorCode = e.code, .errorMessage = e.message}};
}

inline auto Failed::onEvent(UpgradeContext&, const EvReset&)
    -> fsm::TransitionTo<Idle> { return {}; }

// ═══════════════════════════════════════
// FSM type alias
// ═══════════════════════════════════════
using UpgradeFSM = fsm::StateMachine<
    UpgradeContext,
    Idle, Checking, UpgradeAvailable, Downloading,
    Verifying, ReadyToInstall, Installing, Failed
>;

} // namespace ucf::service::upgrade
