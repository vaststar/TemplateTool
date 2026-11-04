#pragma once

/// @file UpgradeStates.h
/// @brief State types and FSM typedef for the Upgrade state machine.

#include "UpgradeEvents.h"
#include "UpgradeContext.h"
#include <ucf/utilities/StateMachineUtils/StateMachine.h>

namespace fsm = ucf::utilities::fsm;

namespace ucf::service::upgrade {

// ── Forward declare all states ──
struct Idle;
struct Checking;
struct UpgradeAvailable;
struct Downloading;
struct Verifying;
struct ReadyToInstall;
struct OpeningInstaller;
struct InstallerOpened;
struct Failed;

// ═══════════════════════════════════════════════════
// State definitions
// ═══════════════════════════════════════════════════

struct Idle
{
    static constexpr std::string_view name()
    {
        return "Idle";
    }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvCheckRequested& e) -> fsm::TransitionTo<Checking>;
};

struct Checking
{
    static constexpr std::string_view name()
    {
        return "Checking";
    }
    bool userTriggered{false};
    void onEnter(UpgradeContext& ctx);
    void onExit(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvCheckSuccess& e)
        -> fsm::OneOf<fsm::TransitionTo<Verifying>, fsm::TransitionTo<UpgradeAvailable>, fsm::Stay>;
    auto onEvent(UpgradeContext& ctx, const EvCheckNoUpgrade& e) -> fsm::OneOf<fsm::TransitionTo<Idle>, fsm::Stay>;
    auto onEvent(UpgradeContext& ctx, const EvError& e) -> fsm::OneOf<fsm::TransitionTo<Failed>, fsm::Stay>;
};

struct UpgradeAvailable
{
    static constexpr std::string_view name()
    {
        return "UpgradeAvailable";
    }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvCheckRequested& e) -> fsm::TransitionTo<Checking>;
    auto onEvent(UpgradeContext& ctx, const EvDownloadStart&)
        -> fsm::OneOf<fsm::TransitionTo<Downloading>, fsm::TransitionTo<Failed>>;
    auto onEvent(UpgradeContext& ctx, const EvDismiss&) -> fsm::TransitionTo<Idle>;
};

struct Downloading
{
    static constexpr std::string_view name()
    {
        return "Downloading";
    }
    void onEnter(UpgradeContext& ctx);
    void onExit(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvProgress& e) -> fsm::Stay;
    auto onEvent(UpgradeContext& ctx, const EvDownloadDone& e)
        -> fsm::OneOf<fsm::TransitionTo<Verifying>, fsm::TransitionTo<Failed>, fsm::Stay>;
    auto onEvent(UpgradeContext& ctx, const EvCancel&) -> fsm::TransitionTo<Idle>;
    auto onEvent(UpgradeContext& ctx, const EvError& e) -> fsm::OneOf<fsm::TransitionTo<Failed>, fsm::Stay>;
};

struct Verifying
{
    static constexpr std::string_view name()
    {
        return "Verifying";
    }
    bool fromCache{false};
    void onEnter(UpgradeContext& ctx);
    void onExit(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvVerifyOk& e) -> fsm::OneOf<fsm::TransitionTo<ReadyToInstall>, fsm::Stay>;
    auto onEvent(UpgradeContext& ctx, const EvError& e)
        -> fsm::OneOf<fsm::TransitionTo<UpgradeAvailable>, fsm::TransitionTo<Failed>, fsm::Stay>;
};

struct ReadyToInstall
{
    static constexpr std::string_view name()
    {
        return "ReadyToInstall";
    }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvOpenInstaller&) -> fsm::TransitionTo<OpeningInstaller>;
    auto onEvent(UpgradeContext& ctx, const EvDismiss&) -> fsm::TransitionTo<Idle>;
};

struct OpeningInstaller
{
    static constexpr std::string_view name()
    {
        return "OpeningInstaller";
    }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvInstallerOpenResult& e)
        -> fsm::OneOf<fsm::TransitionTo<InstallerOpened>, fsm::TransitionTo<ReadyToInstall>>;
};

struct InstallerOpened
{
    static constexpr std::string_view name()
    {
        return "InstallerOpened";
    }
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvDismiss&) -> fsm::TransitionTo<Idle>;
    auto onEvent(UpgradeContext& ctx, const EvCheckRequested& e) -> fsm::TransitionTo<Checking>;
};

struct Failed
{
    static constexpr std::string_view name()
    {
        return "Failed";
    }
    model::UpgradeErrorCode errorCode{};
    std::string errorMessage;
    void onEnter(UpgradeContext& ctx);
    auto onEvent(UpgradeContext& ctx, const EvCheckRequested& e) -> fsm::TransitionTo<Checking>;
    auto onEvent(UpgradeContext& ctx, const EvDismiss&) -> fsm::TransitionTo<Idle>;
};

// ═══════════════════════════════════════════════════
// FSM type alias
// ═══════════════════════════════════════════════════
using UpgradeFSM = fsm::StateMachine<UpgradeContext, Idle, Checking, UpgradeAvailable, Downloading, Verifying,
    ReadyToInstall, OpeningInstaller, InstallerOpened, Failed>;

} // namespace ucf::service::upgrade

// Inline implementations (must be after all state types are complete)
#include "UpgradeTransitions.inl"
