#pragma once

/// @file StateMachine.h
/// @brief C++20 type-safe, thread-safe, event-driven finite state machine.
///
/// Design notes:
///   - States are plain types; no base class is required.
///   - Events are plain types; no common event base is required.
///   - Context is owned by the FSM via std::unique_ptr.
///   - The FSM is thread-safe by default.
///   - Async callbacks should prefer postEvent(...); update() drains the queue.
///   - processEvent(...) is immediate but still serialized and re-entrant safe.

#include "detail/Transitions.h"
#include "detail/Traits.h"
#include "detail/MoveOnlyFunction.h"
#include "detail/Log.h"

#include <cassert>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>

namespace fsm {

// ============================================================================
// StateMachine
// ============================================================================

template <typename Context, typename... States>
class StateMachine
{
    static_assert(sizeof...(States) >= 1, "StateMachine requires at least one state.");
    static_assert((std::is_move_constructible_v<States> && ...),
                  "All states must be move-constructible.");

public:
    using StateVariant = std::variant<States...>;
    using InitialState = std::variant_alternative_t<0, StateVariant>;
    using TransResult = std::variant<Stay, Defer,
                                     SelfTransition, TransitionTo<States>...>;

    using TransitionCB = std::function<void(std::size_t fromIdx, std::size_t toIdx)>;

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
    StateMachine()
        requires(std::default_initializable<Context> && std::default_initializable<InitialState>)
        : m_ctx(std::make_unique<Context>())
        , m_state(InitialState{})
    {
        invokeOnEnter();
    }

    explicit StateMachine(Context ctx)
        requires std::default_initializable<InitialState>
        : m_ctx(std::make_unique<Context>(std::move(ctx)))
        , m_state(InitialState{})
    {
        invokeOnEnter();
    }

    explicit StateMachine(std::unique_ptr<Context> ctx)
        requires std::default_initializable<InitialState>
        : m_ctx(std::move(ctx))
        , m_state(InitialState{})
    {
        assert(m_ctx && "StateMachine requires a non-null Context.");
        invokeOnEnter();
    }

    template <typename... CtxArgs>
    explicit StateMachine(std::in_place_t, CtxArgs&&... ctxArgs)
        requires std::default_initializable<InitialState>
        : m_ctx(std::make_unique<Context>(std::forward<CtxArgs>(ctxArgs)...))
        , m_state(InitialState{})
    {
        invokeOnEnter();
    }

    template <typename InitState>
        requires detail::is_one_of_v<std::decay_t<InitState>, States...>
    StateMachine(Context ctx, InitState init)
        : m_ctx(std::make_unique<Context>(std::move(ctx)))
        , m_state(std::move(init))
    {
        invokeOnEnter();
    }

    template <typename InitState>
        requires detail::is_one_of_v<std::decay_t<InitState>, States...>
    StateMachine(std::unique_ptr<Context> ctx, InitState init)
        : m_ctx(std::move(ctx))
        , m_state(std::move(init))
    {
        assert(m_ctx && "StateMachine requires a non-null Context.");
        invokeOnEnter();
    }

    StateMachine(const StateMachine&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(StateMachine&&) = delete;
    ~StateMachine() = default;

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
    {
        std::lock_guard lock(m_mutex);
        return std::forward<Visitor>(visitor)(*m_ctx);
    }

    template <typename Visitor>
    decltype(auto) withContext(Visitor&& visitor) const
    {
        std::lock_guard lock(m_mutex);
        return std::forward<Visitor>(visitor)(*m_ctx);
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

    void onTransition(TransitionCB cb)
    {
        std::lock_guard lock(m_mutex);
        m_onTransition = std::move(cb);
    }

private:
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
        while (!m_postedActions.empty())
        {
            m_pendingActions.push_back(std::move(m_postedActions.front()));
            m_postedActions.pop_front();
        }
    }

    void replayDeferredEvents()
    {
        while (!m_deferredActions.empty())
        {
            m_pendingActions.push_back(std::move(m_deferredActions.front()));
            m_deferredActions.pop_front();
        }
    }

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
            catch (const std::exception& e)
            {
                FSM_LOG_ERROR("FSM action exception: " << e.what()
                              << " in state: " << currentStateNameUnlocked());
            }
            catch (...)
            {
                FSM_LOG_ERROR("FSM action unknown exception"
                              << " in state: " << currentStateNameUnlocked());
            }
        }
    }

    template <typename Event>
    void processEventImpl(Event&& event)
    {
        using EventT = std::decay_t<Event>;
        const EventT& eventRef = event;

        auto result = std::visit(
            [&](auto& currentState) -> TransResult
            {
                using S = std::decay_t<decltype(currentState)>;
                if constexpr (EventHandler<S, Context, EventT>)
                {
                    auto r = currentState.onEvent(*m_ctx, eventRef);
                    return toTransResult(std::move(r));
                }
                else
                {
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
                    FSM_LOG_DEBUG("FSM event deferred in state: "
                                  << currentStateNameUnlocked());
                    enqueueDeferredEvent(std::forward<Event>(event));
                }
                else if constexpr (std::is_same_v<T, SelfTransition>)
                {
                    const auto idx = m_state.index();
                    const auto name = currentStateNameUnlocked();
                    FSM_LOG_INFO("FSM self-transition: " << name);
                    invokeOnExit();
                    invokeOnEnter();
                    notifyTransition(idx, idx);
                    replayDeferredEvents();
                }
                else
                {
                    const auto fromIdx = m_state.index();
                    const auto fromName = currentStateNameUnlocked();
                    invokeOnExit();

                    using TargetState = typename T::target_state_type;
                    m_state.template emplace<TargetState>(std::move(t.state));

                    const auto toName = currentStateNameUnlocked();
                    FSM_LOG_INFO("FSM transition: "
                                 << fromName << " -> " << toName);
                    invokeOnEnter();
                    notifyTransition(fromIdx, m_state.index());
                    replayDeferredEvents();
                }
            },
            result);
    }

    template <typename TargetState>
    void forceTransitionImpl(TargetState&& newState)
    {
        using StateT = std::decay_t<TargetState>;

        const auto fromIdx = m_state.index();
        const auto fromName = currentStateNameUnlocked();
        invokeOnExit();
        m_state.template emplace<StateT>(std::forward<TargetState>(newState));
        const auto toName = currentStateNameUnlocked();
        FSM_LOG_INFO("FSM force transition: "
                     << fromName << " -> " << toName);
        invokeOnEnter();
        notifyTransition(fromIdx, m_state.index());
        replayDeferredEvents();
    }

    void invokeOnEnter()
    {
        std::visit(
            [this](auto& s)
            {
                using S = std::decay_t<decltype(s)>;
                if constexpr (HasOnEnter<S, Context>)
                    s.onEnter(*m_ctx);
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
                    s.onExit(*m_ctx);
            },
            m_state);
    }

    void notifyTransition(std::size_t from, std::size_t to)
    {
        if (m_onTransition)
            m_onTransition(from, to);
    }

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

private:
    mutable std::recursive_mutex m_mutex;

    std::unique_ptr<Context> m_ctx;
    StateVariant m_state;

    TransitionCB m_onTransition;

    std::deque<Action> m_postedActions;
    std::deque<Action> m_deferredActions;
    std::deque<Action> m_pendingActions;

    bool m_processing = false;
};

} // namespace fsm
