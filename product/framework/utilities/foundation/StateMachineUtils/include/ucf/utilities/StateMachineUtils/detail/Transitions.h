#pragma once

/// @file detail/Transitions.h
/// @brief Transition result types returned by State::onEvent() handlers.

#include <type_traits>
#include <utility>
#include <variant>

namespace ucf::utilities::fsm {

/// Sentinel type indicating no context is needed.
struct NoContext {};

// ============================================================================
// Transition result types
// ============================================================================

/// No state change; the FSM stays in the current state.
struct Stay {};

/// The event is saved and replayed after the next state transition.
struct Defer {};

/// Exit and re-enter the current state (triggers onExit + onEnter).
struct SelfTransition {};

/// Transition to TargetState, optionally carrying a pre-constructed instance.
template <typename TargetState>
struct TransitionTo
{
    using target_state_type = TargetState;

    TargetState state;

    TransitionTo()
        requires std::default_initializable<TargetState>
        : state{}
    {
    }

    explicit TransitionTo(TargetState s) : state(std::move(s)) {}
};

/// Convenience alias: an onEvent handler can return any of these results.
template <typename... Results>
using OneOf = std::variant<Results...>;

} // namespace ucf::utilities::fsm
