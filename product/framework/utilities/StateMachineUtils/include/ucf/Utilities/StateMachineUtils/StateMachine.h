#pragma once

/// @file StateMachine.h
/// @brief C++20 type-safe, thread-safe, event-driven finite state machine.
///
/// Design notes:
///   - States are plain types; no base class is required.
///   - Events are plain types; no common event base is required.
///   - Context is borrowed by reference (FSM does not own it).
///   - Use fsm::NoContext if no shared context is needed.
///   - The FSM is thread-safe by default.
///   - Async callbacks should prefer postEvent(...); update() drains the queue.
///   - processEvent(...) is immediate but still serialized and re-entrant safe.
///
/// Callback constraints (onEnter, onExit, onEvent, onTransition, visitor):
///   - Must be short and non-blocking; they execute under the FSM mutex.
///   - Must not acquire external locks that could also be held when calling the FSM.
///   - Visitors passed to withContext/visitState must not return references or
///     pointers to internal objects; the lock is released when the call returns.

#include "detail/Transitions.h"
#include "detail/Traits.h"
#include "detail/MoveOnlyFunction.h"
#include "detail/Log.h"

#include <cassert>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>

namespace ucf::utilities::fsm {

// ============================================================================
// StateMachine
// ============================================================================

template <typename Context = NoContext, typename... States>
class StateMachine
{
    static_assert(sizeof...(States) >= 1, "StateMachine requires at least one state.");
    static_assert((std::is_move_constructible_v<States> && ...),
                  "All states must be move-constructible.");

    static constexpr bool has_context = !std::is_same_v<Context, NoContext>;

public:
    using StateVariant = std::variant<States...>;
    using InitialState = std::variant_alternative_t<0, StateVariant>;
    using TransResult = std::variant<Stay, Defer,
                                     SelfTransition, TransitionTo<States>...>;

    using TransitionCB = std::function<void(
        std::size_t fromIdx, std::size_t toIdx,
        std::string_view fromName, std::string_view toName)>;
    using UnhandledEventCB = std::function<void(
        std::size_t stateIdx, std::string_view stateName,
        std::string_view eventName)>;

    static constexpr std::size_t state_count = sizeof...(States);

private:
    using Action = detail::MoveOnlyFunction<void(StateMachine&)>;

    struct ProcessingGuard
    {
        explicit ProcessingGuard(bool& flagIn) : flag(flagIn) { flag = true; }
        ~ProcessingGuard() { flag = false; }

        ProcessingGuard(const ProcessingGuard&) = delete;
        ProcessingGuard& operator=(const ProcessingGuard&) = delete;

        bool& flag;
    };

public:
    // ── Constructors: No-Context mode ────────────────────────────────

    StateMachine()
        requires(!has_context && std::default_initializable<InitialState>)
        : m_state(InitialState{})
    {
        invokeOnEnter();
    }

    template <typename InitState>
        requires(!has_context && detail::is_one_of_v<std::decay_t<InitState>, States...>)
    explicit StateMachine(InitState init)
        : m_state(std::move(init))
    {
        invokeOnEnter();
    }

    // ── Constructors: With-Context mode (borrowed reference) ─────────

    explicit StateMachine(Context& ctx)
        requires(has_context && std::default_initializable<InitialState>)
        : m_ctx(&ctx)
        , m_state(InitialState{})
    {
        invokeOnEnter();
    }

    template <typename InitState>
        requires(has_context && detail::is_one_of_v<std::decay_t<InitState>, States...>)
    StateMachine(Context& ctx, InitState init)
        : m_ctx(&ctx)
        , m_state(std::move(init))
    {
        invokeOnEnter();
    }

    StateMachine(const StateMachine&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(StateMachine&&) = delete;

    ~StateMachine()
    {
        try {
            std::lock_guard lock(m_mutex);
            invokeOnExit();
        } catch (...) {
            FSM_LOG_ERROR(logPrefix() << "exception in onExit during destruction");
        }
    }

    // ── Public API: Event Processing ─────────────────────────────────

    template <typename Event>
    void processEvent(Event&& event)
    {
        std::lock_guard lock(m_mutex);
        enqueuePendingEvent(std::forward<Event>(event));
        drainPendingActions();
    }

    template <typename... Events>
    void processEvents(Events&&... events)
    {
        std::lock_guard lock(m_mutex);
        (enqueuePendingEvent(std::forward<Events>(events)), ...);
        drainPendingActions();
    }

    template <typename Event>
    void postEvent(Event&& event)
    {
        std::lock_guard lock(m_mutex);
        enqueuePostedEvent(std::forward<Event>(event));
    }

    template <typename... Events>
    void postEvents(Events&&... events)
    {
        std::lock_guard lock(m_mutex);
        (enqueuePostedEvent(std::forward<Events>(events)), ...);
    }

    void update()
    {
        std::lock_guard lock(m_mutex);
        movePostedToPending();
        drainPendingActions();
    }

    template <typename TargetState>
        requires detail::is_one_of_v<std::decay_t<TargetState>, States...>
    void forceTransition(TargetState&& newState)
    {
        std::lock_guard lock(m_mutex);
        using StateT = std::decay_t<TargetState>;
        m_pendingActions.emplace_back(
            [state = StateT(std::forward<TargetState>(newState))](StateMachine& sm) mutable {
                sm.forceTransitionImpl(std::move(state));
            });
        drainPendingActions();
    }

    void reset()
        requires std::default_initializable<InitialState>
    {
        forceTransition(InitialState{});
    }

    // ── Public API: State Introspection ──────────────────────────────

    template <typename S>
    [[nodiscard]] bool isIn() const
    {
        std::lock_guard lock(m_mutex);
        return std::holds_alternative<S>(m_state);
    }

    [[nodiscard]] std::size_t stateIndex() const
    {
        std::lock_guard lock(m_mutex);
        return m_state.index();
    }

    template <typename S>
        requires detail::is_one_of_v<S, States...>
    [[nodiscard]] static constexpr std::size_t indexOf() noexcept
    {
        return detail::index_of<S, States...>::value;
    }

    template <typename Event>
    [[nodiscard]] bool canHandle() const
    {
        std::lock_guard lock(m_mutex);
        return std::visit(
            [](const auto& s) -> bool
            {
                using S = std::decay_t<decltype(s)>;
                return EventHandler<S, Context, Event>;
            },
            m_state);
    }

    [[nodiscard]] std::string_view currentStateName() const
    {
        std::lock_guard lock(m_mutex);
        return currentStateNameUnlocked();
    }

    template <typename S>
    [[nodiscard]] static constexpr std::string_view stateName() noexcept
    {
        if constexpr (HasStateName<S>)
            return S::name();
        else
            return detail::type_name<S>();
    }

    [[nodiscard]] std::size_t deferredCount() const
    {
        std::lock_guard lock(m_mutex);
        return m_deferredActions.size();
    }

    [[nodiscard]] std::size_t queuedCount() const
    {
        std::lock_guard lock(m_mutex);
        return m_postedActions.size();
    }

    void clearDeferred()
    {
        std::lock_guard lock(m_mutex);
        m_deferredActions.clear();
    }

    template <typename Visitor>
    decltype(auto) withContext(Visitor&& visitor)
        requires has_context
    {
        std::lock_guard lock(m_mutex);
        return std::forward<Visitor>(visitor)(*m_ctx);
    }

    template <typename Visitor>
    decltype(auto) withContext(Visitor&& visitor) const
        requires has_context
    {
        std::lock_guard lock(m_mutex);
        return std::forward<Visitor>(visitor)(std::as_const(*m_ctx));
    }

    template <typename Visitor>
    decltype(auto) visitState(Visitor&& visitor)
    {
        std::lock_guard lock(m_mutex);
        return std::visit(std::forward<Visitor>(visitor), m_state);
    }

    template <typename Visitor>
    decltype(auto) visitState(Visitor&& visitor) const
    {
        std::lock_guard lock(m_mutex);
        return std::visit(std::forward<Visitor>(visitor), m_state);
    }

    // ── Public API: Callbacks & Configuration ────────────────────────

    void onTransition(TransitionCB cb)
    {
        std::lock_guard lock(m_mutex);
        m_onTransition = std::move(cb);
    }

    void onUnhandledEvent(UnhandledEventCB cb)
    {
        std::lock_guard lock(m_mutex);
        m_onUnhandledEvent = std::move(cb);
    }

    void setName(std::string name)
    {
        std::lock_guard lock(m_mutex);
        m_name = std::move(name);
        updateLogPrefix();
    }

    [[nodiscard]] std::string name() const
    {
        std::lock_guard lock(m_mutex);
        return m_name;
    }

private:
    // ── Private: Event Queue Management ─────────────────────────────

    template <typename Event>
    void enqueuePendingEvent(Event&& event)
    {
        using EventT = std::decay_t<Event>;
        m_pendingActions.emplace_back(
            [evt = EventT(std::forward<Event>(event))](StateMachine& sm) mutable {
                sm.processEventImpl(std::move(evt));
            });
    }

    template <typename Event>
    void enqueuePostedEvent(Event&& event)
    {
        using EventT = std::decay_t<Event>;
        m_postedActions.emplace_back(
            [evt = EventT(std::forward<Event>(event))](StateMachine& sm) mutable {
                sm.processEventImpl(std::move(evt));
            });
    }

    template <typename Event>
    void enqueueDeferredEvent(Event&& event)
    {
        using EventT = std::decay_t<Event>;
        m_deferredActions.emplace_back(
            [evt = EventT(std::forward<Event>(event))](StateMachine& sm) mutable {
                sm.enqueuePendingEvent(std::move(evt));
            });
    }

    void movePostedToPending()
    {
        m_pendingActions.insert(m_pendingActions.end(),
            std::make_move_iterator(m_postedActions.begin()),
            std::make_move_iterator(m_postedActions.end()));
        m_postedActions.clear();
    }

    void replayDeferredEvents()
    {
        m_pendingActions.insert(m_pendingActions.end(),
            std::make_move_iterator(m_deferredActions.begin()),
            std::make_move_iterator(m_deferredActions.end()));
        m_deferredActions.clear();
    }

    // ── Private: Event Drain ─────────────────────────────────────────

    void drainPendingActions()
    {
        if (m_processing)
            return;

        ProcessingGuard guard(m_processing);

        while (!m_pendingActions.empty())
        {
            auto action = std::move(m_pendingActions.front());
            m_pendingActions.pop_front();
            try
            {
                action(*this);
            }
            catch (const std::exception& ex)
            {
                (void)ex;
                FSM_LOG_ERROR(logPrefix() << "action exception: " << ex.what()
                              << " in state: " << currentStateNameUnlocked());
                m_pendingActions.clear();
                break;
            }
            catch (...)
            {
                FSM_LOG_ERROR(logPrefix() << "action unknown exception"
                              << " in state: " << currentStateNameUnlocked());
                m_pendingActions.clear();
                break;
            }
        }
    }

    // ── Private: Event Dispatch ──────────────────────────────────────
    template <typename S, typename Event>
    auto dispatchEvent(S& state, const Event& event)
    {
        if constexpr (has_context)
            return state.onEvent(*m_ctx, event);
        else
            return state.onEvent(event);
    }
    template <typename Event>
    void processEventImpl(Event&& event)
    {
        using EventT = std::decay_t<Event>;
        const EventT& eventRef = event;

        FSM_LOG_DEBUG(logPrefix() << "processing event: "
                      << detail::type_name<EventT>()
                      << " in state: " << currentStateNameUnlocked());

        auto result = std::visit(
            [&](auto& currentState) -> TransResult
            {
                using S = std::decay_t<decltype(currentState)>;
                if constexpr (EventHandler<S, Context, EventT>)
                {
                    static_assert(ValidEventHandler<S, Context, EventT>,
                        "onEvent() return type must be Stay, Defer, SelfTransition, "
                        "TransitionTo<S>, or OneOf<...> of these types.");
                    auto r = dispatchEvent(currentState, eventRef);
                    return toTransResult(std::move(r));
                }
                else
                {
                    FSM_LOG_WARN(logPrefix() << "unhandled event: "
                                 << detail::type_name<EventT>()
                                 << " in state: " << currentStateNameUnlocked());
                    if (m_onUnhandledEvent)
                        m_onUnhandledEvent(m_state.index(),
                                           currentStateNameUnlocked(),
                                           detail::type_name<EventT>());
                    return Stay{};
                }
            },
            m_state);

        applyTransition(result, std::forward<Event>(event));
    }

    template <typename R>
    TransResult toTransResult(R&& r)
    {
        using Raw = std::decay_t<R>;

        if constexpr (std::is_same_v<Raw, Stay>)
        {
            return Stay{};
        }
        else if constexpr (std::is_same_v<Raw, Defer>)
        {
            return Defer{};
        }
        else if constexpr (std::is_same_v<Raw, SelfTransition>)
        {
            return SelfTransition{};
        }
        else if constexpr (detail::is_variant_v<Raw>)
        {
            return std::visit(
                [this](auto&& alt) -> TransResult
                {
                    return toTransResult(std::move(alt));
                },
                std::forward<R>(r));
        }
        else
        {
            return TransResult{std::forward<R>(r)};
        }
    }

    // ── Private: Transition Execution ────────────────────────────────

    template <typename EmplaceFn>
    void executeTransition(EmplaceFn&& emplace, [[maybe_unused]] std::string_view label)
    {
        const auto fromIdx = m_state.index();
        const auto fromName = currentStateNameUnlocked();
        invokeOnExit();

        std::forward<EmplaceFn>(emplace)();

        const auto toIdx = m_state.index();
        const auto toName = currentStateNameUnlocked();
        FSM_LOG_INFO(logPrefix() << label << ": "
                     << fromName << " -> " << toName);
        invokeOnEnter();
        notifyTransition(fromIdx, toIdx, fromName, toName);
        replayDeferredEvents();
    }

    template <typename Event>
    void applyTransition(TransResult& result, Event&& event)
    {
        std::visit(
            [&](auto& t)
            {
                using T = std::decay_t<decltype(t)>;

                if constexpr (std::is_same_v<T, Stay>)
                {
                }
                else if constexpr (std::is_same_v<T, Defer>)
                {
                    FSM_LOG_DEBUG(logPrefix() << "event deferred in state: "
                                  << currentStateNameUnlocked());
                    enqueueDeferredEvent(std::forward<Event>(event));
                }
                else if constexpr (std::is_same_v<T, SelfTransition>)
                {
                    executeTransition([]{}, "self-transition");
                }
                else
                {
                    using TargetState = typename T::target_state_type;
                    executeTransition(
                        [&]{ m_state.template emplace<TargetState>(std::move(t.state)); },
                        "transition");
                }
            },
            result);
    }

    template <typename TargetState>
    void forceTransitionImpl(TargetState&& newState)
    {
        using StateT = std::decay_t<TargetState>;
        executeTransition(
            [&]{ m_state.template emplace<StateT>(std::forward<TargetState>(newState)); },
            "force transition");
    }

    // ── Private: Lifecycle Hooks ─────────────────────────────────────

    void invokeOnEnter()
    {
        std::visit(
            [this](auto& s)
            {
                using S = std::decay_t<decltype(s)>;
                if constexpr (HasOnEnter<S, Context>)
                {
                    if constexpr (has_context)
                        s.onEnter(*m_ctx);
                    else
                        s.onEnter();
                }
            },
            m_state);
    }

    void invokeOnExit()
    {
        std::visit(
            [this](auto& s)
            {
                using S = std::decay_t<decltype(s)>;
                if constexpr (HasOnExit<S, Context>)
                {
                    if constexpr (has_context)
                        s.onExit(*m_ctx);
                    else
                        s.onExit();
                }
            },
            m_state);
    }

    void notifyTransition(std::size_t from, std::size_t to,
                          std::string_view fromName, std::string_view toName)
    {
        if (m_onTransition)
            m_onTransition(from, to, fromName, toName);
    }

    // ── Private: Utilities ──────────────────────────────────────────

    [[nodiscard]] std::string_view currentStateNameUnlocked() const
    {
        return std::visit(
            [](const auto& s) -> std::string_view
            {
                using S = std::decay_t<decltype(s)>;
                if constexpr (HasStateName<S>)
                    return S::name();
                else
                    return detail::type_name<S>();
            },
            m_state);
    }

    [[nodiscard]] const std::string& logPrefix() const noexcept
    {
        return m_logPrefix;
    }

    void updateLogPrefix()
    {
        if (m_name.empty())
            m_logPrefix = "FSM ";
        else
            m_logPrefix = "FSM[" + m_name + "] ";
    }

private:
    // ── Private: Data Members ───────────────────────────────────────

    mutable std::recursive_mutex m_mutex;

    Context* m_ctx = nullptr;
    StateVariant m_state;
    std::string m_name;
    std::string m_logPrefix = "FSM ";

    TransitionCB m_onTransition;
    UnhandledEventCB m_onUnhandledEvent;

    std::deque<Action> m_postedActions;
    std::deque<Action> m_deferredActions;
    std::deque<Action> m_pendingActions;

    bool m_processing = false;
};

} // namespace ucf::utilities::fsm
