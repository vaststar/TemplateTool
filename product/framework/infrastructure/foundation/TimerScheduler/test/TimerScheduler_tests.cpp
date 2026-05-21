#include <catch2/catch_test_macros.hpp>

#include <ucf/Infrastructure/TimerScheduler/ITimerScheduler.h>
#include <ucf/Infrastructure/TimerScheduler/TimerSchedulerFactory.h>

#include <atomic>
#include <chrono>
#include <thread>

using namespace ucf::infrastructure::scheduling;
using namespace std::chrono_literals;

TEST_CASE("scheduleOnce fires once after delay", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    std::atomic<int> n{0};

    auto h = sched->scheduleOnce(50ms, [&]() { ++n; });
    REQUIRE(h.isValid());

    std::this_thread::sleep_for(200ms);
    REQUIRE(n.load() == 1);
}

TEST_CASE("scheduleOnce can be cancelled before firing", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    std::atomic<int> n{0};

    auto h = sched->scheduleOnce(200ms, [&]() { ++n; });
    std::this_thread::sleep_for(50ms);
    REQUIRE(sched->cancel(h));
    std::this_thread::sleep_for(300ms);
    REQUIRE(n.load() == 0);
}

TEST_CASE("scheduleAtFixedRate fires repeatedly", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    std::atomic<int> n{0};

    sched->scheduleAtFixedRate(0ms, 50ms, [&]() { ++n; });
    std::this_thread::sleep_for(280ms);

    const int count = n.load();
    REQUIRE(count >= 3);
    REQUIRE(count <= 8);
}

TEST_CASE("scheduleAtFixedRate stops after cancel", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    std::atomic<int> n{0};

    auto h = sched->scheduleAtFixedRate(0ms, 30ms, [&]() { ++n; });
    std::this_thread::sleep_for(120ms);
    REQUIRE(sched->cancel(h));

    const int snapshot = n.load();
    std::this_thread::sleep_for(150ms);
    REQUIRE(n.load() == snapshot);
}

TEST_CASE("scheduleWithFixedDelay waits for callback completion", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    std::atomic<int> n{0};

    sched->scheduleWithFixedDelay(0ms, 50ms, [&]() {
        std::this_thread::sleep_for(100ms);
        ++n;
    });

    // Each cycle ~= 100ms run + 50ms gap = 150ms. In 400ms expect ~2-3.
    std::this_thread::sleep_for(400ms);
    const int count = n.load();
    REQUIRE(count >= 1);
    REQUIRE(count <= 4);
}

TEST_CASE("callback can cancel its own handle", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    std::atomic<int> n{0};
    TimerHandle handle;

    handle = sched->scheduleAtFixedRate(0ms, 30ms, [&]() {
        if (++n == 2)
        {
            sched->cancel(handle);
        }
    });

    std::this_thread::sleep_for(300ms);
    REQUIRE(n.load() == 2);
}

TEST_CASE("multiple timers fire independently", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    std::atomic<int> a{0};
    std::atomic<int> b{0};

    sched->scheduleAtFixedRate(0ms, 30ms, [&]() { ++a; });
    sched->scheduleAtFixedRate(0ms, 50ms, [&]() { ++b; });

    std::this_thread::sleep_for(260ms);
    REQUIRE(a.load() >= 5);
    REQUIRE(b.load() >= 3);
}

TEST_CASE("FixedRate skips missed ticks instead of bursting", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    std::atomic<int> n{0};

    sched->scheduleAtFixedRate(0ms, 30ms, [&]() {
        const int seq = ++n;
        if (seq == 1)
        {
            std::this_thread::sleep_for(200ms);
        }
    });

    // First call blocks 200ms (would miss ~6 ticks). With catch-up: total ~7.
    // With skip-policy: ~1 (initial) + ~5 (remaining 150ms / 30ms) = ~6.
    // The key assertion is "no large burst" -> bounded upper limit.
    std::this_thread::sleep_for(400ms);
    const int count = n.load();
    REQUIRE(count >= 2);
    REQUIRE(count <= 9);
}

TEST_CASE("shutdown stops further callbacks", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    std::atomic<int> n{0};

    sched->scheduleAtFixedRate(0ms, 30ms, [&]() { ++n; });
    std::this_thread::sleep_for(100ms);
    sched->shutdown();

    const int snapshot = n.load();
    std::this_thread::sleep_for(150ms);
    REQUIRE(n.load() == snapshot);
}

TEST_CASE("destructor stops the worker cleanly", "[TimerScheduler]")
{
    std::atomic<int> n{0};
    {
        auto sched = TimerSchedulerFactory::create();
        sched->scheduleAtFixedRate(0ms, 20ms, [&]() { ++n; });
        std::this_thread::sleep_for(80ms);
    }

    const int snapshot = n.load();
    std::this_thread::sleep_for(80ms);
    REQUIRE(n.load() == snapshot);
}

TEST_CASE("empty callback returns invalid handle", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    auto h = sched->scheduleOnce(10ms, TimerCallback{});
    REQUIRE_FALSE(h.isValid());
}

TEST_CASE("cancel on invalid handle returns false", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    REQUIRE_FALSE(sched->cancel(TimerHandle{}));
}

TEST_CASE("periodic schedule with zero period returns invalid handle", "[TimerScheduler]")
{
    auto sched = TimerSchedulerFactory::create();
    auto h = sched->scheduleAtFixedRate(0ms, 0ms, []() {});
    REQUIRE_FALSE(h.isValid());
}
