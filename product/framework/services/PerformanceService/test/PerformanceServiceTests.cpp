#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/PerformanceService/IPerformanceService.h>
#include <ucf/services/PerformanceService/PerformanceServiceCreator.h>
#include <ucf/utilities/JsonUtils/JsonValue.h>
#include <ucf/utilities/TimeUtils/Instant.h>
#include "PerformanceCpuCalculator.h"
#include "PerformanceMonitoringSchedule.h"
#include "TimingTracker.h"

#include <chrono>
#include <optional>
#include <thread>
#include <vector>

TEST_CASE("Performance monitoring schedule keeps absolute sample cadence",
          "[PerformanceService][MonitoringSchedule]")
{
    using namespace std::chrono_literals;
    using Schedule = ucf::service::PerformanceMonitoringSchedule;

    const Schedule::TimePoint startTime{};
    Schedule schedule(startTime, 1s, 30s);

    REQUIRE(schedule.nextSampleDeadline() == startTime + 1s);

    schedule.advanceSampleDeadline(startTime + 1s + 25ms);
    REQUIRE(schedule.nextSampleDeadline() == startTime + 2s);

    schedule.advanceSampleDeadline(startTime + 2s);
    REQUIRE(schedule.nextSampleDeadline() == startTime + 3s);
}

TEST_CASE("Performance monitoring schedule skips elapsed slots without catch-up bursts",
          "[PerformanceService][MonitoringSchedule]")
{
    using namespace std::chrono_literals;
    using Schedule = ucf::service::PerformanceMonitoringSchedule;

    const Schedule::TimePoint startTime{};
    Schedule schedule(startTime, 1s, 30s);

    schedule.advanceSampleDeadline(startTime + 5s + 400ms);
    REQUIRE(schedule.nextSampleDeadline() == startTime + 6s);

    REQUIRE_FALSE(schedule.consumeReportDeadline(startTime + 29s + 999ms));
    REQUIRE(schedule.consumeReportDeadline(startTime + 30s + 25ms));
    REQUIRE(schedule.nextReportDeadline() == startTime + 60s);

    REQUIRE(schedule.consumeReportDeadline(startTime + 125s));
    REQUIRE(schedule.nextReportDeadline() == startTime + 150s);
    REQUIRE_FALSE(schedule.consumeReportDeadline(startTime + 125s));
}

TEST_CASE("Performance monitoring schedule rejects non-positive intervals",
          "[PerformanceService][MonitoringSchedule]")
{
    using namespace std::chrono_literals;
    using Schedule = ucf::service::PerformanceMonitoringSchedule;

    const Schedule::TimePoint startTime{};
    REQUIRE_THROWS_AS(Schedule(startTime, 0s, 30s), std::invalid_argument);
    REQUIRE_THROWS_AS(Schedule(startTime, 1s, 0s), std::invalid_argument);
}

TEST_CASE("PerformanceService creation", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto performanceService = ucf::service::impl::createPerformanceService(fakeCoreFramework);
    REQUIRE(performanceService != nullptr);
    REQUIRE(performanceService->getServiceName() == "PerformanceService");
    REQUIRE_FALSE(performanceService->getProcessCpuUsagePercent().has_value());
    REQUIRE_FALSE(performanceService->getSystemCpuUsagePercent().has_value());
}

TEST_CASE("PerformanceService memory monitoring", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = ucf::service::impl::createPerformanceService(fakeCoreFramework);

    auto memInfo = performanceService->getCurrentMemoryUsage();

    // Resident memory is a common metric supported by all target platforms.
    REQUIRE(memInfo.processResidentBytes.has_value());
    REQUIRE(*memInfo.processResidentBytes > 0);
    REQUIRE(memInfo.processPeakResidentBytes.has_value());
    REQUIRE(*memInfo.processPeakResidentBytes >= *memInfo.processResidentBytes);
    REQUIRE(memInfo.systemAvailablePhysicalBytes.has_value());
    REQUIRE(*memInfo.systemAvailablePhysicalBytes > 0);

#if defined(_WIN32)
    REQUIRE(memInfo.processPrivateCommittedBytes.has_value());
    REQUIRE_FALSE(memInfo.processVirtualAddressSpaceBytes.has_value());
#else
    REQUIRE(memInfo.processVirtualAddressSpaceBytes.has_value());
    REQUIRE_FALSE(memInfo.processPrivateCommittedBytes.has_value());
#endif
}

TEST_CASE("PerformanceService memory threshold", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = ucf::service::impl::createPerformanceService(fakeCoreFramework);

    REQUIRE(performanceService->getProcessResidentMemoryWarningThreshold() == 0);

    performanceService->setProcessResidentMemoryWarningThreshold(500 * 1024 * 1024);
    REQUIRE(
        performanceService->getProcessResidentMemoryWarningThreshold() ==
        500 * 1024 * 1024);
}

TEST_CASE("TimingTracker basic timing", "[PerformanceService][TimingTracker]")
{
    using namespace std::chrono_literals;

    ucf::service::TimingTracker tracker;

    auto token = tracker.beginTiming("TestOperation");
    std::this_thread::sleep_for(10ms);
    tracker.endTiming(token);

    const auto stats = tracker.getStats("TestOperation");
    REQUIRE(stats);
    REQUIRE(stats->operationName == "TestOperation");
    REQUIRE(stats->callCount == 1);
    REQUIRE(stats->totalDuration >= 10ms);
    REQUIRE(stats->minimumDuration == stats->totalDuration);
    REQUIRE(stats->maximumDuration == stats->totalDuration);
    REQUIRE(stats->averageDuration() == stats->totalDuration);
}

TEST_CASE("TimingTracker multiple calls", "[PerformanceService][TimingTracker]")
{
    using namespace std::chrono_literals;

    ucf::service::TimingTracker tracker;

    for (int i = 0; i < 5; ++i)
    {
        auto token = tracker.beginTiming("MultiCall");
        std::this_thread::sleep_for(5ms);
        tracker.endTiming(token);
    }

    const auto stats = tracker.getStats("MultiCall");
    REQUIRE(stats);
    REQUIRE(stats->callCount == 5);
    REQUIRE(stats->totalDuration >= 25ms);
    REQUIRE(stats->minimumDuration);
    REQUIRE(stats->maximumDuration);
    REQUIRE(stats->averageDuration());
}

TEST_CASE("TimingTracker getAllStats returns stable operation-name order",
          "[PerformanceService][TimingTracker]")
{
    ucf::service::TimingTracker tracker;

    auto token1 = tracker.beginTiming("ZuluOperation");
    tracker.endTiming(token1);

    auto token2 = tracker.beginTiming("AlphaOperation");
    tracker.endTiming(token2);

    const auto allStats = tracker.getAllStats();
    REQUIRE(allStats.size() == 2);
    REQUIRE(allStats[0].operationName == "AlphaOperation");
    REQUIRE(allStats[1].operationName == "ZuluOperation");
}

TEST_CASE("TimingTracker reset clears statistics and invalidates active tokens",
          "[PerformanceService][TimingTracker]")
{
    ucf::service::TimingTracker tracker;

    auto completedToken = tracker.beginTiming("CompletedBeforeReset");
    tracker.endTiming(completedToken);
    auto activeToken = tracker.beginTiming("ActiveDuringReset");

    REQUIRE(tracker.getAllStats().size() == 1);

    tracker.reset();
    tracker.endTiming(activeToken);

    REQUIRE(tracker.getAllStats().empty());
    REQUIRE_FALSE(tracker.getStats("ActiveDuringReset"));
}

TEST_CASE("TimingTracker records sub-millisecond durations in microseconds",
          "[PerformanceService][TimingTracker]")
{
    using namespace std::chrono_literals;

    ucf::service::TimingTracker tracker;
    auto token = tracker.beginTiming("SubMillisecond");
    std::this_thread::sleep_for(500us);
    tracker.endTiming(token);

    const auto stats = tracker.getStats("SubMillisecond");
    REQUIRE(stats);
    REQUIRE(stats->totalDuration >= 500us);
}

TEST_CASE("TimingTracker consumes each token only once",
          "[PerformanceService][TimingTracker]")
{
    ucf::service::TimingTracker tracker;
    const auto token = tracker.beginTiming("CompleteOnce");

    tracker.endTiming(token);
    tracker.endTiming(token);

    const auto stats = tracker.getStats("CompleteOnce");
    REQUIRE(stats);
    REQUIRE(stats->callCount == 1);
}

TEST_CASE("TimingTracker rejects tokens from another tracker",
          "[PerformanceService][TimingTracker]")
{
    ucf::service::TimingTracker firstTracker;
    ucf::service::TimingTracker secondTracker;
    const auto token = firstTracker.beginTiming("OwnedByFirstTracker");

    secondTracker.endTiming(token);
    REQUIRE_FALSE(secondTracker.getStats("OwnedByFirstTracker"));

    firstTracker.endTiming(token);
    REQUIRE(firstTracker.getStats("OwnedByFirstTracker"));
}

TEST_CASE("TimingTracker rejects empty operation names",
          "[PerformanceService][TimingTracker]")
{
    ucf::service::TimingTracker tracker;
    const auto token = tracker.beginTiming("");

    REQUIRE_FALSE(token.isValid());
    REQUIRE(tracker.getAllStats().empty());
    REQUIRE_FALSE(tracker.getStats(""));
}

TEST_CASE("TimingTracker supports concurrent samples",
          "[PerformanceService][TimingTracker]")
{
    constexpr int threadCount = 4;
    constexpr int samplesPerThread = 10;

    ucf::service::TimingTracker tracker;
    std::vector<std::thread> workers;
    workers.reserve(threadCount);

    for (int threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        workers.emplace_back([&tracker]
        {
            for (int sampleIndex = 0; sampleIndex < samplesPerThread; ++sampleIndex)
            {
                const auto token = tracker.beginTiming("ConcurrentOperation");
                tracker.endTiming(token);
            }
        });
    }

    for (auto& worker : workers)
    {
        worker.join();
    }

    const auto stats = tracker.getStats("ConcurrentOperation");
    REQUIRE(stats);
    REQUIRE(stats->callCount == threadCount * samplesPerThread);
}

TEST_CASE("PerformanceService snapshot", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = ucf::service::impl::createPerformanceService(fakeCoreFramework);

    // Add some timing data
    auto token = performanceService->beginTiming("SnapshotTest");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    performanceService->endTiming(token);

    auto snapshot = performanceService->takeSnapshot();

    REQUIRE(snapshot.memory.processResidentBytes.has_value());
    REQUIRE(*snapshot.memory.processResidentBytes > 0);
    REQUIRE(snapshot.timingStats.size() == 1);
    REQUIRE(snapshot.timingStats[0].operationName == "SnapshotTest");
}

TEST_CASE("PerformanceService JSON export", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = ucf::service::impl::createPerformanceService(fakeCoreFramework);

    auto token = performanceService->beginTiming("JsonTest");
    performanceService->endTiming(token);

    auto json = performanceService->exportReportAsJson();

    REQUIRE(!json.empty());
    const auto parsed = ucf::utilities::JsonValue::parseEx(json);
    REQUIRE(parsed.ok());

    const auto timestamp = parsed.value.get("timestamp").asString();
    REQUIRE(timestamp);
    REQUIRE(ucf::utilities::Instant::parseRfc3339(timestamp.value()));
    const auto memory = parsed.value.get("memory");
    REQUIRE(memory.isObject());
    REQUIRE(memory.contains("processResidentBytes"));
    REQUIRE(memory.contains("processPeakResidentBytes"));
    REQUIRE(memory.contains("processVirtualAddressSpaceBytes"));
    REQUIRE(memory.contains("processPrivateCommittedBytes"));
    REQUIRE(memory.contains("systemAvailablePhysicalBytes"));
#if defined(_WIN32)
    REQUIRE(memory.get("processVirtualAddressSpaceBytes").isNull());
#else
    REQUIRE(memory.get("processPrivateCommittedBytes").isNull());
#endif
    REQUIRE(json.find("processCpuUsagePercent") != std::string::npos);
    REQUIRE(json.find("systemCpuUsagePercent") != std::string::npos);
    const auto timingStats = parsed.value.get("timingStats");
    REQUIRE(timingStats.isArray());
    REQUIRE(timingStats.size() == 1);
    const auto timing = timingStats.get(static_cast<size_t>(0));
    REQUIRE(timing.get("operation").asString() == "JsonTest");
    REQUIRE(timing.contains("totalDurationMicroseconds"));
    REQUIRE(timing.contains("averageDurationMicroseconds"));
    REQUIRE(timing.contains("minimumDurationMicroseconds"));
    REQUIRE(timing.contains("maximumDurationMicroseconds"));
    REQUIRE_FALSE(timing.contains("totalMs"));
}

TEST_CASE("ScopedTiming RAII", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = ucf::service::impl::createPerformanceService(fakeCoreFramework);

    {
        ucf::service::ScopedTiming timing(performanceService, "ScopedTest");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    const auto stats = performanceService->getTimingStats("ScopedTest");
    REQUIRE(stats);
    REQUIRE(stats->callCount == 1);
    REQUIRE(stats->totalDuration >= std::chrono::milliseconds(5));
}

TEST_CASE("PerformanceCpuCalculator establishes baselines before reporting usage",
          "[PerformanceService][CPU]")
{
    using Clock = std::chrono::steady_clock;

    ucf::service::PerformanceCpuCalculator calculator;
    const auto sample = calculator.update(
        ucf::service::ProcessCpuTime{500000},
        ucf::service::SystemCpuTimes{.busyTicks = 100, .totalTicks = 1000},
        Clock::time_point{});

    REQUIRE(sample.process.status == ucf::service::CpuSampleStatus::BaselineEstablished);
    REQUIRE_FALSE(sample.process.percent.has_value());
    REQUIRE(sample.system.status == ucf::service::CpuSampleStatus::BaselineEstablished);
    REQUIRE_FALSE(sample.system.percent.has_value());
}

TEST_CASE("PerformanceCpuCalculator uses one logical core as 100 percent",
          "[PerformanceService][CPU]")
{
    using Clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    ucf::service::PerformanceCpuCalculator calculator;
    const auto start = Clock::time_point{};
    static_cast<void>(calculator.update(
        ucf::service::ProcessCpuTime{0},
        ucf::service::SystemCpuTimes{.busyTicks = 100, .totalTicks = 1000},
        start));

    const auto oneCoreSample = calculator.update(
        ucf::service::ProcessCpuTime{1000000},
        ucf::service::SystemCpuTimes{.busyTicks = 300, .totalTicks = 2000},
        start + 1s);
    REQUIRE(oneCoreSample.process.percent.value() == Catch::Approx(100.0));
    REQUIRE(oneCoreSample.system.percent.value() == Catch::Approx(20.0));

    const auto twoCoreSample = calculator.update(
        ucf::service::ProcessCpuTime{3000000},
        ucf::service::SystemCpuTimes{.busyTicks = 500, .totalTicks = 3000},
        start + 2s);
    REQUIRE(twoCoreSample.process.percent.value() == Catch::Approx(200.0));
    REQUIRE(twoCoreSample.system.percent.value() == Catch::Approx(20.0));
}

TEST_CASE("PerformanceCpuCalculator rejects a backwards process counter and resets its baseline",
          "[PerformanceService][CPU]")
{
    using Clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    ucf::service::PerformanceCpuCalculator calculator;
    const auto start = Clock::time_point{};
    static_cast<void>(calculator.update(
        ucf::service::ProcessCpuTime{5000000},
        std::nullopt,
        start));

    const auto resetSample = calculator.update(
        ucf::service::ProcessCpuTime{4000000},
        std::nullopt,
        start + 1s);
    REQUIRE(resetSample.process.status == ucf::service::CpuSampleStatus::CounterReset);
    REQUIRE_FALSE(resetSample.process.percent.has_value());
    REQUIRE(resetSample.process.previousTime.count() == 5000000);
    REQUIRE(resetSample.process.currentTime.count() == 4000000);

    const auto recoveredSample = calculator.update(
        ucf::service::ProcessCpuTime{5000000},
        std::nullopt,
        start + 2s);
    REQUIRE(recoveredSample.process.status == ucf::service::CpuSampleStatus::Valid);
    REQUIRE(recoveredSample.process.percent.value() == Catch::Approx(100.0));
}

TEST_CASE("PerformanceCpuCalculator preserves its baseline across an unavailable sample",
          "[PerformanceService][CPU]")
{
    using Clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    ucf::service::PerformanceCpuCalculator calculator;
    const auto start = Clock::time_point{};
    static_cast<void>(calculator.update(
        ucf::service::ProcessCpuTime{1000000},
        std::nullopt,
        start));

    const auto unavailableSample = calculator.update(std::nullopt, std::nullopt, start + 1s);
    REQUIRE(unavailableSample.process.status == ucf::service::CpuSampleStatus::Unavailable);
    REQUIRE_FALSE(unavailableSample.process.percent.has_value());

    const auto recoveredSample = calculator.update(
        ucf::service::ProcessCpuTime{3000000},
        std::nullopt,
        start + 2s);
    REQUIRE(recoveredSample.process.percent.value() == Catch::Approx(100.0));
}

TEST_CASE("PerformanceCpuCalculator rejects invalid system counter deltas",
          "[PerformanceService][CPU]")
{
    using Clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    ucf::service::PerformanceCpuCalculator calculator;
    const auto start = Clock::time_point{};
    static_cast<void>(calculator.update(
        std::nullopt,
        ucf::service::SystemCpuTimes{.busyTicks = 100, .totalTicks = 1000},
        start));

    const auto invalidSample = calculator.update(
        std::nullopt,
        ucf::service::SystemCpuTimes{.busyTicks = 1200, .totalTicks = 2000},
        start + 1s);
    REQUIRE(invalidSample.system.status == ucf::service::CpuSampleStatus::Invalid);
    REQUIRE_FALSE(invalidSample.system.percent.has_value());
}

TEST_CASE("CpuUsageWindow averages only valid samples", "[PerformanceService][CPU]")
{
    ucf::service::CpuUsageWindow window;
    window.add(100.0);
    window.add(std::nullopt);
    window.add(200.0);

    REQUIRE(window.sampleCount() == 2);
    REQUIRE(window.average().value() == Catch::Approx(150.0));

    window.reset();
    REQUIRE(window.sampleCount() == 0);
    REQUIRE_FALSE(window.average().has_value());
}
