#include <catch2/catch_test_macros.hpp>

#include <ucf/Utilities/StateMachineUtils/StateMachine.h>

#include <array>
#include <string>
#include <thread>
#include <vector>

namespace fsm = ucf::utilities::fsm;

// ============================================================================
// Test fixtures — minimal context, states, events
// ============================================================================

namespace {

struct Ctx
{
    int enterCount  = 0;
    int exitCount   = 0;
    int value       = 0;
};

// Events
struct EvGo   {};
struct EvBack {};
struct EvTick { int n = 1; };
struct EvCfg  { int val; };
struct EvBoom {};
struct EvMaybe { bool ok = true; };

// States — forward declarations
struct Idle;
struct Active;
struct Done;

// ------- Idle -------
struct Idle
{
    static constexpr std::string_view name() { return "Idle"; }

    void onEnter(Ctx& ctx) { ++ctx.enterCount; }
    void onExit(Ctx& ctx)  { ++ctx.exitCount; }

    // Declared here, defined after all states are complete.
    auto onEvent(Ctx&, const EvGo&) -> fsm::TransitionTo<Active>;
    auto onEvent(Ctx&, const EvCfg&) -> fsm::Defer { return {}; }
};

// ------- Active -------
struct Active
{
    static constexpr std::string_view name() { return "Active"; }

    void onEnter(Ctx& ctx) { ++ctx.enterCount; }
    void onExit(Ctx& ctx)  { ++ctx.exitCount; }

    auto onEvent(Ctx&, const EvBack&) -> fsm::TransitionTo<Idle>;
    auto onEvent(Ctx&, const EvGo&) -> fsm::TransitionTo<Done>;

    auto onEvent(Ctx& ctx, const EvTick& e) -> fsm::Stay
    {
        ctx.value += e.n;
        return {};
    }

    auto onEvent(Ctx& ctx, const EvBoom&) -> fsm::SelfTransition
    {
        ctx.value = 0;
        return {};
    }

    auto onEvent(Ctx& ctx, const EvCfg& e) -> fsm::Stay
    {
        ctx.value = e.val;
        return {};
    }

    auto onEvent(Ctx&, const EvMaybe& e)
        -> fsm::OneOf<fsm::TransitionTo<Done>, fsm::Stay>;
};

// ------- Done -------
struct Done
{
    static constexpr std::string_view name() { return "Done"; }
    void onEnter(Ctx& ctx) { ++ctx.enterCount; }

    auto onEvent(Ctx&, const EvBack&) -> fsm::TransitionTo<Idle>;
};

// Out-of-line definitions (all target types are now complete)
inline auto Idle::onEvent(Ctx&, const EvGo&) -> fsm::TransitionTo<Active> { return {}; }
inline auto Active::onEvent(Ctx&, const EvBack&) -> fsm::TransitionTo<Idle> { return {}; }
inline auto Active::onEvent(Ctx&, const EvGo&) -> fsm::TransitionTo<Done> { return {}; }
inline auto Active::onEvent(Ctx&, const EvMaybe& e)
    -> fsm::OneOf<fsm::TransitionTo<Done>, fsm::Stay>
{
    if (e.ok)
        return fsm::TransitionTo<Done>{};
    return fsm::Stay{};
}
inline auto Done::onEvent(Ctx&, const EvBack&) -> fsm::TransitionTo<Idle> { return {}; }

using TestFSM = fsm::StateMachine<Ctx, Idle, Active, Done>;

} // anonymous namespace

// ============================================================================
// Tests
// ============================================================================

TEST_CASE("FSM starts in first state", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    REQUIRE(sm.isIn<Idle>());
    REQUIRE(sm.stateIndex() == 0);
    REQUIRE(sm.currentStateName() == "Idle");
}

TEST_CASE("Basic transition Idle -> Active -> Done", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    sm.processEvent(EvGo{});
    REQUIRE(sm.isIn<Active>());

    sm.processEvent(EvGo{});
    REQUIRE(sm.isIn<Done>());
}

TEST_CASE("onEnter and onExit are called", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    // Idle.onEnter called on construction
    REQUIRE(ctx.enterCount == 1); // Idle.onEnter
    REQUIRE(ctx.exitCount == 0);

    sm.processEvent(EvGo{}); // Idle -> Active: Idle.onExit + Active.onEnter
    REQUIRE(ctx.enterCount == 2); // +Active.onEnter
    REQUIRE(ctx.exitCount == 1);  // +Idle.onExit
}

TEST_CASE("Stay does not trigger onExit/onEnter", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    sm.processEvent(EvGo{}); // -> Active

    int enterBefore = ctx.enterCount;
    int exitBefore  = ctx.exitCount;

    sm.processEvent(EvTick{5});

    REQUIRE(ctx.enterCount == enterBefore); // no onEnter
    REQUIRE(ctx.exitCount == exitBefore);   // no onExit
    REQUIRE(ctx.value == 5);

    REQUIRE(sm.isIn<Active>());
}

TEST_CASE("SelfTransition triggers onExit + onEnter", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    sm.processEvent(EvGo{}); // -> Active

    int enterBefore = ctx.enterCount;
    int exitBefore  = ctx.exitCount;

    sm.processEvent(EvBoom{}); // self-transition in Active

    REQUIRE(ctx.enterCount == enterBefore + 1);
    REQUIRE(ctx.exitCount == exitBefore + 1);
    REQUIRE(ctx.value == 0); // reset by EvBoom handler

    REQUIRE(sm.isIn<Active>());
}

TEST_CASE("Deferred events replay after transition", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);

    // EvCfg is deferred in Idle
    sm.processEvent(EvCfg{42});
    REQUIRE(sm.deferredCount() == 1);

    // Transition to Active replays the deferred EvCfg
    sm.processEvent(EvGo{});
    REQUIRE(sm.isIn<Active>());
    REQUIRE(sm.deferredCount() == 0);

    REQUIRE(ctx.value == 42);
}

TEST_CASE("clearDeferred removes deferred events", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    sm.processEvent(EvCfg{99});
    REQUIRE(sm.deferredCount() == 1);

    sm.clearDeferred();
    REQUIRE(sm.deferredCount() == 0);

    sm.processEvent(EvGo{});
    REQUIRE(ctx.value == 0); // config was cleared, not replayed
}

TEST_CASE("OneOf conditional transition", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    sm.processEvent(EvGo{}); // -> Active

    SECTION("condition true -> transitions to Done")
    {
        sm.processEvent(EvMaybe{true});
        REQUIRE(sm.isIn<Done>());
    }

    SECTION("condition false -> stays in Active")
    {
        sm.processEvent(EvMaybe{false});
        REQUIRE(sm.isIn<Active>());
    }
}

TEST_CASE("forceTransition bypasses event handlers", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    sm.forceTransition(Done{});
    REQUIRE(sm.isIn<Done>());
}

TEST_CASE("reset returns to initial state", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    sm.processEvent(EvGo{});
    REQUIRE(sm.isIn<Active>());

    sm.reset();
    REQUIRE(sm.isIn<Idle>());
}

TEST_CASE("canHandle checks event support at compile+runtime", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    // Idle handles EvGo but not EvTick
    REQUIRE(sm.canHandle<EvGo>());
    REQUIRE_FALSE(sm.canHandle<EvTick>());

    sm.processEvent(EvGo{}); // -> Active
    REQUIRE(sm.canHandle<EvTick>());
    REQUIRE(sm.canHandle<EvGo>());
}

TEST_CASE("indexOf returns correct compile-time index", "[fsm]")
{
    STATIC_REQUIRE(TestFSM::indexOf<Idle>() == 0);
    STATIC_REQUIRE(TestFSM::indexOf<Active>() == 1);
    STATIC_REQUIRE(TestFSM::indexOf<Done>() == 2);
}

TEST_CASE("stateName returns name for named states", "[fsm]")
{
    REQUIRE(TestFSM::stateName<Idle>() == "Idle");
    REQUIRE(TestFSM::stateName<Active>() == "Active");
    REQUIRE(TestFSM::stateName<Done>() == "Done");
}

TEST_CASE("state_count is correct", "[fsm]")
{
    STATIC_REQUIRE(TestFSM::state_count == 3);
}

TEST_CASE("visitState dispatches to current state", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);

    std::string_view visited;
    sm.visitState([&](const auto& s) {
        using S = std::decay_t<decltype(s)>;
        if constexpr (fsm::HasStateName<S>)
            visited = S::name();
    });
    REQUIRE(visited == "Idle");
}

TEST_CASE("withContext provides mutable and const access", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);

    sm.withContext([](Ctx& c) { c.value = 77; });
    sm.withContext([](const Ctx& c) { REQUIRE(c.value == 77); });
}

TEST_CASE("processEvents handles batch of events", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    sm.processEvents(EvGo{}, EvTick{10}, EvTick{20});

    REQUIRE(sm.isIn<Active>());
    REQUIRE(ctx.value == 30);
}

TEST_CASE("postEvent + update defers processing until update", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);

    sm.postEvent(EvGo{});
    REQUIRE(sm.isIn<Idle>()); // not processed yet
    REQUIRE(sm.queuedCount() == 1);

    sm.update();
    REQUIRE(sm.isIn<Active>());
    REQUIRE(sm.queuedCount() == 0);
}

TEST_CASE("postEvents queues multiple events", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);

    sm.postEvents(EvGo{}, EvTick{5});
    REQUIRE(sm.queuedCount() == 2);

    sm.update();
    REQUIRE(sm.isIn<Active>());
    REQUIRE(ctx.value == 5);
}

TEST_CASE("Transition callback fires on state change", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);

    struct TransInfo { std::size_t from; std::size_t to; std::string fromName; std::string toName; };
    std::vector<TransInfo> transitions;
    sm.onTransition([&](std::size_t from, std::size_t to,
                        std::string_view fromName, std::string_view toName) {
        transitions.push_back({from, to, std::string(fromName), std::string(toName)});
    });

    sm.processEvent(EvGo{});   // Idle(0) -> Active(1)
    sm.processEvent(EvGo{});   // Active(1) -> Done(2)

    REQUIRE(transitions.size() == 2);
    REQUIRE(transitions[0].from == 0);
    REQUIRE(transitions[0].to == 1);
    REQUIRE(transitions[0].fromName == "Idle");
    REQUIRE(transitions[0].toName == "Active");
    REQUIRE(transitions[1].from == 1);
    REQUIRE(transitions[1].to == 2);
    REQUIRE(transitions[1].fromName == "Active");
    REQUIRE(transitions[1].toName == "Done");
}

TEST_CASE("Unhandled event callback fires for unhandled events", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);

    struct UnhandledInfo { std::size_t idx; std::string state; std::string event; };
    std::vector<UnhandledInfo> unhandled;
    sm.onUnhandledEvent([&](std::size_t idx, std::string_view stateName,
                            std::string_view eventName) {
        unhandled.push_back({idx, std::string(stateName), std::string(eventName)});
    });

    // Idle does not handle EvTick
    sm.processEvent(EvTick{});
    REQUIRE(unhandled.size() == 1);
    REQUIRE(unhandled[0].idx == 0);
    REQUIRE(unhandled[0].state == "Idle");
    REQUIRE(unhandled[0].event.find("EvTick") != std::string::npos);

    // Idle does not handle EvBoom
    sm.processEvent(EvBoom{});
    REQUIRE(unhandled.size() == 2);
    REQUIRE(unhandled[1].event.find("EvBoom") != std::string::npos);

    // Transition to Active, EvTick is now handled
    sm.processEvent(EvGo{});
    sm.processEvent(EvTick{});
    REQUIRE(unhandled.size() == 2); // no new unhandled
}

TEST_CASE("Custom initial state via constructor", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx, Active{});
    REQUIRE(sm.isIn<Active>());
}

TEST_CASE("Thread safety - concurrent postEvent + update", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    sm.processEvent(EvGo{}); // -> Active

    constexpr int numThreads = 4;
    constexpr int eventsPerThread = 100;

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([&sm] {
            for (int i = 0; i < eventsPerThread; ++i)
            {
                sm.postEvent(EvTick{1});
            }
        });
    }

    for (auto& t : threads)
        t.join();

    sm.update();

    sm.withContext([](const Ctx& c) {
        REQUIRE(c.value == numThreads * eventsPerThread);
    });
}

TEST_CASE("Thread safety - concurrent processEvent", "[fsm]")
{
    Ctx ctx{};
    TestFSM sm(ctx);
    sm.processEvent(EvGo{}); // -> Active

    constexpr int numThreads = 4;
    constexpr int eventsPerThread = 50;

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([&sm] {
            for (int i = 0; i < eventsPerThread; ++i)
            {
                sm.processEvent(EvTick{1});
            }
        });
    }

    for (auto& t : threads)
        t.join();

    sm.withContext([](const Ctx& c) {
        REQUIRE(c.value == numThreads * eventsPerThread);
    });
}

// ============================================================================
// MoveOnlyFunction tests
// ============================================================================

TEST_CASE("MoveOnlyFunction - basic callable", "[fsm][MoveOnlyFunction]")
{
    fsm::detail::MoveOnlyFunction<int()> fn([] { return 42; });
    REQUIRE(fn);
    REQUIRE(fn() == 42);
}

TEST_CASE("MoveOnlyFunction - move semantics", "[fsm][MoveOnlyFunction]")
{
    fsm::detail::MoveOnlyFunction<int()> fn([] { return 7; });
    auto fn2 = std::move(fn);
    REQUIRE_FALSE(fn);
    REQUIRE(fn2);
    REQUIRE(fn2() == 7);
}

TEST_CASE("MoveOnlyFunction - nullptr", "[fsm][MoveOnlyFunction]")
{
    fsm::detail::MoveOnlyFunction<void()> fn(nullptr);
    REQUIRE_FALSE(fn);
}

TEST_CASE("MoveOnlyFunction - heap fallback for large captures", "[fsm][MoveOnlyFunction]")
{
    // Capture enough data to exceed the 64-byte SBO buffer
    std::array<char, 128> bigData{};
    bigData.fill('X');

    fsm::detail::MoveOnlyFunction<char()> fn([bigData] { return bigData[0]; });
    REQUIRE(fn);
    REQUIRE(fn() == 'X');

    auto fn2 = std::move(fn);
    REQUIRE(fn2() == 'X');
}

// ============================================================================
// NoContext tests
// ============================================================================

namespace {

struct LightOff;
struct LightOn;

struct LightOff
{
    static constexpr std::string_view name() { return "LightOff"; }
    auto onEvent(const EvGo&) -> fsm::TransitionTo<LightOn>;
};

struct LightOn
{
    static constexpr std::string_view name() { return "LightOn"; }
    void onEnter() {}
    void onExit() {}
    auto onEvent(const EvBack&) -> fsm::TransitionTo<LightOff>;
};

inline auto LightOff::onEvent(const EvGo&) -> fsm::TransitionTo<LightOn> { return {}; }
inline auto LightOn::onEvent(const EvBack&) -> fsm::TransitionTo<LightOff> { return {}; }

using NoCtxFSM = fsm::StateMachine<fsm::NoContext, LightOff, LightOn>;

} // anonymous namespace

TEST_CASE("NoContext FSM basic transitions", "[fsm][nocontext]")
{
    NoCtxFSM sm;
    REQUIRE(sm.isIn<LightOff>());

    sm.processEvent(EvGo{});
    REQUIRE(sm.isIn<LightOn>());

    sm.processEvent(EvBack{});
    REQUIRE(sm.isIn<LightOff>());
}

TEST_CASE("NoContext FSM state_count and names", "[fsm][nocontext]")
{
    STATIC_REQUIRE(NoCtxFSM::state_count == 2);
    REQUIRE(NoCtxFSM::stateName<LightOff>() == "LightOff");
    REQUIRE(NoCtxFSM::stateName<LightOn>() == "LightOn");
}

TEST_CASE("NoContext FSM custom initial state", "[fsm][nocontext]")
{
    NoCtxFSM sm(LightOn{});
    REQUIRE(sm.isIn<LightOn>());
}

TEST_CASE("NoContext FSM reset", "[fsm][nocontext]")
{
    NoCtxFSM sm;
    sm.processEvent(EvGo{});
    REQUIRE(sm.isIn<LightOn>());

    sm.reset();
    REQUIRE(sm.isIn<LightOff>());
}
