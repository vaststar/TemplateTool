#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/PerformanceService/IPerformanceService.h>
#include <ucf/services/PerformanceService/PerformanceServiceCreator.h>
#include <ucf/utilities/JsonUtils/JsonValue.h>
#include <ucf/utilities/TimeUtils/Instant.h>
#include "PerformanceCpuCalculator.h"
#include "TimingTracker.h"

#include <chrono>
#include <optional>
#include <thread>

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
    ucf::service::TimingTracker tracker;

    auto token = tracker.beginTiming("TestOperation");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    tracker.endTiming(token);

    auto stats = tracker.getStats("TestOperation");
    REQUIRE(stats.operationName == "TestOperation");
    REQUIRE(stats.callCount == 1);
    REQUIRE(stats.totalTime.count() >= 10);
}

TEST_CASE("TimingTracker multiple calls", "[PerformanceService][TimingTracker]")
{
    ucf::service::TimingTracker tracker;

    for (int i = 0; i < 5; ++i)
    {
        auto token = tracker.beginTiming("MultiCall");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        tracker.endTiming(token);
    }

    auto stats = tracker.getStats("MultiCall");
    REQUIRE(stats.callCount == 5);
    REQUIRE(stats.totalTime.count() >= 25);
}

TEST_CASE("TimingTracker getAllStats", "[PerformanceService][TimingTracker]")
{
    ucf::service::TimingTracker tracker;

    auto token1 = tracker.beginTiming("Op1");
    tracker.endTiming(token1);

    auto token2 = tracker.beginTiming("Op2");
    tracker.endTiming(token2);

    auto allStats = tracker.getAllStats();
    REQUIRE(allStats.size() == 2);
}

TEST_CASE("TimingTracker reset", "[PerformanceService][TimingTracker]")
{
    ucf::service::TimingTracker tracker;

    auto token = tracker.beginTiming("ToBeReset");
    tracker.endTiming(token);

    REQUIRE(tracker.getAllStats().size() == 1);

    tracker.reset();

    REQUIRE(tracker.getAllStats().empty());
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
    REQUIRE(json.find("JsonTest") != std::string::npos);
}

TEST_CASE("ScopedTiming RAII", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = ucf::service::impl::createPerformanceService(fakeCoreFramework);

    {
        ucf::service::ScopedTiming timing(performanceService, "ScopedTest");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    auto stats = performanceService->getTimingStats("ScopedTest");
    REQUIRE(stats.callCount == 1);
    REQUIRE(stats.totalTime.count() >= 5);
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
