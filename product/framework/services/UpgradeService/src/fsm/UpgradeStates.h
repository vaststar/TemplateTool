#pragma once

/// @file UpgradeStates.h
/// @brief State types and FSM typedef for the Upgrade state machine.

#include "UpgradeEvents.h"
#include "UpgradeContext.h"
#include <ucf/Utilities/StateMachineUtils/StateMachine.h>

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

// ═══════════════════════════════════════════════════
// State definitions
// ═══════════════════════════════════════════════════

struct Idle {
    static constexpr std::string_view name() { return "Idle"; }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvCheckRequested& e)
        -> fsm::TransitionTo<Checking>;
};

struct Checking {
    static constexpr std::string_view name() { return "Checking"; }
    bool userTriggered{false};
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvCheckSuccess& e)
        -> fsm::TransitionTo<UpgradeAvailable>;
    auto onEvent(UpgradeContext& ctx, const EvCheckNoUpgrade&)
        -> fsm::TransitionTo<Idle>;
    auto onEvent(UpgradeContext& ctx, const EvError& e)
        -> fsm::TransitionTo<Failed>;
};

struct UpgradeAvailable {
    static constexpr std::string_view name() { return "UpgradeAvailable"; }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvDownloadStart&)
        -> fsm::TransitionTo<Downloading>;
    auto onEvent(UpgradeContext& ctx, const EvRemindLater&)
        -> fsm::TransitionTo<Idle>;
};

struct Downloading {
    static constexpr std::string_view name() { return "Downloading"; }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvProgress& e) -> fsm::Stay;
    auto onEvent(UpgradeContext& ctx, const EvDownloadDone& e)
        -> fsm::TransitionTo<Verifying>;
    auto onEvent(UpgradeContext& ctx, const EvCancel&)
        -> fsm::TransitionTo<Idle>;
    auto onEvent(UpgradeContext& ctx, const EvError& e)
        -> fsm::TransitionTo<Failed>;
};

struct Verifying {
    static constexpr std::string_view name() { return "Verifying"; }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvVerifyOk&)
        -> fsm::TransitionTo<ReadyToInstall>;
    auto onEvent(UpgradeContext& ctx, const EvError& e)
        -> fsm::TransitionTo<Failed>;
};

struct ReadyToInstall {
    static constexpr std::string_view name() { return "ReadyToInstall"; }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvInstallStart&)
        -> fsm::TransitionTo<Installing>;
};

struct Installing {
    static constexpr std::string_view name() { return "Installing"; }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvError& e)
        -> fsm::TransitionTo<Failed>;
};

struct Failed {
    static constexpr std::string_view name() { return "Failed"; }
    model::UpgradeErrorCode errorCode{};
    std::string errorMessage;
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvReset&)
        -> fsm::TransitionTo<Idle>;
};

// ═══════════════════════════════════════════════════
// FSM type alias
// ═══════════════════════════════════════════════════
using UpgradeFSM = fsm::StateMachine<
    UpgradeContext,
    Idle, Checking, UpgradeAvailable, Downloading,
    Verifying, ReadyToInstall, Installing, Failed
>;

} // namespace ucf::service::upgrade

// Inline implementations (must be after all state types are complete)
#include "UpgradeTransitions.inl"
