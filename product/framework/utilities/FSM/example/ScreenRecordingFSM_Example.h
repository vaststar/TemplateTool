/// @file ScreenRecordingFSM_Example.h
/// @brief Comprehensive example showcasing ALL features of the FSM library.
///
/// Demonstrates:
///   1. Basic transitions (TransitionTo)
///   2. Lifecycle hooks (onEnter / onExit)
///   3. Conditional transitions (OneOf)
///   4. Internal transitions (handle event, no exit/enter)
///   5. Self-transitions (exit + re-enter same state)
///   6. Deferred events (save for later)
///   7. Transition & unhandled-event callbacks
///   8. State introspection (isIn, canHandle, stateIndex, stateName)
///   9. Force transitions & reset
///  10. Re-entrant safety (events emitted during onEnter)
///  11. Built-in thread safety + queued async events
///  12. Minimal async callback integration pattern

#pragma once

#include <ucf/Utilities/FSM/StateMachine.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace example {

// ============================================================================
//  1. Context — shared data for all states
// ============================================================================

struct RecordingContext
{
    std::string outputPath;
    std::string ffmpegPath;
    int duration       = 0;
    int tickCount      = 0;
    bool isGifMode     = false;
    bool hasPermission = true;
    bool ffmpegReady   = true;
};

// ============================================================================
//  2. Events
// ============================================================================

struct StartEvent   { std::string path; std::string format; };
struct StartedEvent {};
struct StopEvent    {};
struct StoppedEvent {};
struct PauseEvent   {};
struct ResumeEvent  {};
struct AbortEvent   {};
struct TickEvent    {};                 // for InternalTransition demo
struct ResetEvent   {};                 // for SelfTransition demo
struct ConfigEvent  { int newDuration; };  // for Defer demo
struct AsyncReadyEvent { int duration; };
struct ErrorEvent   { std::string message; };

// ============================================================================
//  3. States — each state is a plain struct; only handle what you care about
// ============================================================================

// Forward declarations
struct IdleState;
struct StartingState;
struct RecordingState;
struct PausedState;
struct StoppingState;
struct ErrorState;

// ---------- Idle -----------------------------------------------------------

struct IdleState
{
    static constexpr std::string_view name() { return "Idle"; }

    void onEnter(RecordingContext& ctx)
    {
        ctx.duration = 0;
        ctx.tickCount = 0;
        ctx.isGifMode = false;
        std::cout << "[Idle] entered\n";
    }

    /// Basic transition: Idle → Starting
    auto onEvent(RecordingContext& ctx, const StartEvent& e)
        -> fsm::TransitionTo<StartingState>
    {
        ctx.outputPath = e.path;
        ctx.isGifMode = (e.format == "gif");
        std::cout << "[Idle] → Starting (path=" << e.path << ")\n";
        return {};
    }

    /// DEFER demo: can't apply config while idle, save for the next state.
    auto onEvent(RecordingContext&, const ConfigEvent&) -> fsm::Defer
    {
        std::cout << "[Idle] deferring ConfigEvent for later\n";
        return {};
    }
};

// ---------- Starting -------------------------------------------------------

struct StartingState
{
    static constexpr std::string_view name() { return "Starting"; }

    void onEnter(RecordingContext&)
    {
        std::cout << "[Starting] entered — launching async work...\n";
    }

    /// ONEOF demo: runtime-conditional transition
    auto onEvent(RecordingContext& ctx, const StartedEvent&)
        -> fsm::OneOf<fsm::TransitionTo<RecordingState>,
                       fsm::TransitionTo<ErrorState>,
                       fsm::Stay>
    {
        if (!ctx.hasPermission)
        {
            std::cout << "[Starting] → Error (no permission)\n";
            return fsm::TransitionTo<ErrorState>{ErrorState{"no permission"}};
        }
        if (!ctx.ffmpegReady)
        {
            std::cout << "[Starting] staying (FFmpeg not ready yet)\n";
            return fsm::Stay{};
        }
        std::cout << "[Starting] → Recording\n";
        return fsm::TransitionTo<RecordingState>{};
    }

    auto onEvent(RecordingContext&, const StopEvent&)
        -> fsm::TransitionTo<StoppingState>
    {
        std::cout << "[Starting] → Stopping (early stop)\n";
        return {};
    }

    auto onEvent(RecordingContext&, const ErrorEvent& e)
        -> fsm::TransitionTo<ErrorState>
    {
        return fsm::TransitionTo<ErrorState>{ErrorState{e.message}};
    }

    auto onEvent(RecordingContext& ctx, const AsyncReadyEvent& e)
        -> fsm::TransitionTo<RecordingState>
    {
        ctx.duration = e.duration;
        std::cout << "[Starting] async callback applied duration=" << e.duration << "\n";
        return {};
    }
};

// ---------- Recording ------------------------------------------------------

struct RecordingState
{
    static constexpr std::string_view name() { return "Recording"; }

    void onEnter(RecordingContext& ctx)
    {
        std::cout << "[Recording] entered (deferred events will replay now)\n";
        // If a ConfigEvent was deferred, it will be replayed automatically.
        (void)ctx;
    }

    void onExit(RecordingContext&)
    {
        std::cout << "[Recording] exiting\n";
    }

    auto onEvent(RecordingContext&, const StopEvent&)
        -> fsm::TransitionTo<StoppingState>
    {
        std::cout << "[Recording] → Stopping\n";
        return {};
    }

    auto onEvent(RecordingContext&, const PauseEvent&)
        -> fsm::TransitionTo<PausedState>
    {
        std::cout << "[Recording] → Paused\n";
        return {};
    }

    /// INTERNAL TRANSITION demo: handle event without exit/enter.
    auto onEvent(RecordingContext& ctx, const TickEvent&)
        -> fsm::InternalTransition
    {
        ctx.tickCount++;
        // No onExit/onEnter called — just update context silently.
        return {};
    }

    /// SELF TRANSITION demo: re-enter Recording (triggers onExit + onEnter).
    auto onEvent(RecordingContext& ctx, const ResetEvent&)
        -> fsm::SelfTransition
    {
        ctx.tickCount = 0;
        std::cout << "[Recording] self-resetting\n";
        return {};
    }

    /// Handle the deferred ConfigEvent when we're finally in Recording.
    auto onEvent(RecordingContext& ctx, const ConfigEvent& e)
        -> fsm::InternalTransition
    {
        ctx.duration = e.newDuration;
        std::cout << "[Recording] applied deferred config: duration=" << e.newDuration << "\n";
        return {};
    }
};

// ---------- Paused ---------------------------------------------------------

struct PausedState
{
    static constexpr std::string_view name() { return "Paused"; }

    void onEnter(RecordingContext&) { std::cout << "[Paused] entered\n"; }

    auto onEvent(RecordingContext&, const ResumeEvent&)
        -> fsm::TransitionTo<RecordingState>
    {
        std::cout << "[Paused] → Recording\n";
        return {};
    }

    auto onEvent(RecordingContext&, const StopEvent&)
        -> fsm::TransitionTo<StoppingState>
    {
        std::cout << "[Paused] → Stopping\n";
        return {};
    }
};

// ---------- Stopping -------------------------------------------------------

struct StoppingState
{
    static constexpr std::string_view name() { return "Stopping"; }

    void onEnter(RecordingContext&)
    {
        std::cout << "[Stopping] entered — waiting for cleanup...\n";
    }

    auto onEvent(RecordingContext&, const StoppedEvent&)
        -> fsm::TransitionTo<IdleState>
    {
        std::cout << "[Stopping] → Idle\n";
        return {};
    }

    auto onEvent(RecordingContext&, const ErrorEvent& e)
        -> fsm::TransitionTo<ErrorState>
    {
        return fsm::TransitionTo<ErrorState>{ErrorState{e.message}};
    }
};

// ---------- Error ----------------------------------------------------------

struct ErrorState
{
    static constexpr std::string_view name() { return "Error"; }

    std::string reason;

    void onEnter(RecordingContext&)
    {
        std::cout << "[Error] entered: " << reason << "\n";
    }

    /// From Error, only a StopEvent can bring us back to Idle.
    auto onEvent(RecordingContext&, const StopEvent&)
        -> fsm::TransitionTo<IdleState>
    {
        std::cout << "[Error] → Idle (recovering)\n";
        return {};
    }
};

// ============================================================================
//  4. Type alias
// ============================================================================

using RecordFSM = fsm::StateMachine<
    RecordingContext,
    IdleState,       // ← initial state (first in list)
    StartingState,
    RecordingState,
    PausedState,
    StoppingState,
    ErrorState
>;

// ============================================================================
//  5. Demo — exercises every feature
// ============================================================================

inline void demo()
{
    std::cout << "══════════════════════════════════════\n";
    std::cout << " FSM Feature Demo\n";
    std::cout << "══════════════════════════════════════\n\n";

    RecordFSM sm(RecordingContext{});

    // ── Transition callback ──
    sm.onTransition([&](std::size_t from, std::size_t to) {
        std::cout << "  [callback] transition: index " << from << " → " << to
                  << " (now in: " << sm.currentStateName() << ")\n";
    });

    // ── Unhandled event callback ──
    sm.onUnhandledEvent([&](std::size_t idx) {
        std::cout << "  [callback] unhandled event in state index " << idx << "\n";
    });

    // ── 1. Basic transition ──
    std::cout << "--- 1. Basic transition ---\n";
    sm.processEvent(StartEvent{"/tmp/out.mp4", "mp4"});
    sm.processEvent(StartedEvent{});   // Starting → Recording

    // ── 2. Internal transition (no exit/enter) ──
    std::cout << "\n--- 2. InternalTransition (TickEvent) ---\n";
    sm.processEvent(TickEvent{});
    sm.processEvent(TickEvent{});
    sm.processEvent(TickEvent{});
    sm.withContext([](const RecordingContext& ctx) {
        std::cout << "  tickCount = " << ctx.tickCount << "\n";
    });

    // ── 3. Self transition (exit + re-enter) ──
    std::cout << "\n--- 3. SelfTransition (ResetEvent) ---\n";
    sm.processEvent(ResetEvent{});
    sm.withContext([](const RecordingContext& ctx) {
        std::cout << "  tickCount after reset = " << ctx.tickCount << "\n";
    });

    // ── 4. Regular transition chain ──
    std::cout << "\n--- 4. Pause / Resume / Stop ---\n";
    sm.processEvent(PauseEvent{});
    sm.processEvent(ResumeEvent{});
    sm.processEvent(StopEvent{});
    sm.processEvent(StoppedEvent{});
    std::cout << "  isIn<IdleState>? " << sm.isIn<IdleState>() << "\n";

    // ── 5. Deferred events ──
    std::cout << "\n--- 5. Deferred events ---\n";
    // Send ConfigEvent while in Idle → it gets deferred.
    sm.processEvent(ConfigEvent{120});
    std::cout << "  deferred count = " << sm.deferredCount() << "\n";
    // Now transition to Recording — deferred ConfigEvent auto-replays.
    sm.processEvent(StartEvent{"/tmp/out2.mp4", "mp4"});
    sm.processEvent(StartedEvent{});
    sm.withContext([](const RecordingContext& ctx) {
        std::cout << "  duration (from deferred config) = " << ctx.duration << "\n";
    });
    std::cout << "  deferred count = " << sm.deferredCount() << "\n";

    // ── 6. OneOf with runtime guard ──
    std::cout << "\n--- 6. OneOf guard (no permission) ---\n";
    sm.processEvent(StopEvent{});
    sm.processEvent(StoppedEvent{});
    sm.withContext([](RecordingContext& ctx) { ctx.hasPermission = false; });
    sm.processEvent(StartEvent{"/tmp/out3.mp4", "mp4"});
    sm.processEvent(StartedEvent{});  // → ErrorState
    std::cout << "  isIn<ErrorState>? " << sm.isIn<ErrorState>() << "\n";
    sm.processEvent(StopEvent{});     // Error → Idle
    sm.withContext([](RecordingContext& ctx) { ctx.hasPermission = true; });

    // ── 7. Unhandled event ──
    std::cout << "\n--- 7. Unhandled event ---\n";
    sm.processEvent(PauseEvent{});  // Idle doesn't handle Pause

    // ── 8. State introspection ──
    std::cout << "\n--- 8. State introspection ---\n";
    std::cout << "  stateIndex    = " << sm.stateIndex() << "\n";
    std::cout << "  stateName     = " << sm.currentStateName() << "\n";
    std::cout << "  canHandle<Start>? " << sm.canHandle<StartEvent>() << "\n";
    std::cout << "  canHandle<Pause>? " << sm.canHandle<PauseEvent>() << "\n";
    std::cout << "  indexOf<Idle> = " << RecordFSM::indexOf<IdleState>() << "\n";
    std::cout << "  indexOf<Rec>  = " << RecordFSM::indexOf<RecordingState>() << "\n";

    // visitState
    sm.visitState([](const auto& s) {
        using S = std::decay_t<decltype(s)>;
        if constexpr (fsm::HasStateName<S>)
            std::cout << "  visitState: " << S::name() << "\n";
    });

    // ── 9. Force transition ──
    std::cout << "\n--- 9. Force transition ---\n";
    sm.forceTransition(RecordingState{});
    std::cout << "  isIn<Recording>? " << sm.isIn<RecordingState>() << "\n";
    sm.reset();  // back to IdleState
    std::cout << "  isIn<Idle> after reset? " << sm.isIn<IdleState>() << "\n";

    // ── 10. Built-in thread safety + queued async style ──
    std::cout << "\n--- 10. Built-in thread safety + postEvent/update ---\n";
    RecordFSM asyncSm(RecordingContext{});
    asyncSm.postEvent(StartEvent{"/tmp/ts.mp4", "mp4"});
    asyncSm.postEvent(StartedEvent{});
    std::cout << "  queued before update = " << asyncSm.queuedCount() << "\n";
    asyncSm.update();
    std::cout << "  isIn<Recording>? " << asyncSm.isIn<RecordingState>() << "\n";
    std::cout << "  stateName = " << asyncSm.currentStateName() << "\n";
    asyncSm.withContext([](const RecordingContext& ctx) {
        std::cout << "  withContext: tickCount=" << ctx.tickCount
                  << ", duration=" << ctx.duration << "\n";
    });

    // ── 11. Minimal async callback integration ──
    std::cout << "\n--- 11. Minimal async callback integration ---\n";
    RecordFSM callbackSm(RecordingContext{});
    callbackSm.processEvent(StartEvent{"/tmp/async.mp4", "mp4"});

    std::thread worker([&callbackSm] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Real async callback pattern:
        //   1. do not touch old state objects directly
        //   2. do not assume the FSM is still in the old state
        //   3. only post an event back to the FSM
        callbackSm.postEvent(AsyncReadyEvent{888});
    });

    worker.join();

    std::cout << "  queued before async update = " << callbackSm.queuedCount() << "\n";
    callbackSm.update();
    std::cout << "  isIn<Recording>? " << callbackSm.isIn<RecordingState>() << "\n";
    callbackSm.withContext([](const RecordingContext& ctx) {
        std::cout << "  async duration = " << ctx.duration << "\n";
    });

    std::cout << "\n══════════════════════════════════════\n";
    std::cout << " All features demonstrated ✓\n";
    std::cout << "══════════════════════════════════════\n";
}

} // namespace example
