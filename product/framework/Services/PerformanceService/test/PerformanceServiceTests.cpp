#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "PerformanceService.h"
#include "PerformanceManager.h"
#include "TimingTracker.h"

#include <thread>
#include <chrono>

using namespace ucf::service;

TEST_CASE("PerformanceService creation", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto performanceService = std::make_shared<PerformanceService>(fakeCoreFramework);
    REQUIRE(performanceService != nullptr);
    REQUIRE(performanceService->getServiceName() == "PerformanceService");
}

TEST_CASE("PerformanceService memory monitoring", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = std::make_shared<PerformanceService>(fakeCoreFramework);
    
    auto memInfo = performanceService->getCurrentMemoryUsage();
    
    // Process should use some memory
    REQUIRE(memInfo.physicalBytes > 0);
}

TEST_CASE("PerformanceService memory threshold", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = std::make_shared<PerformanceService>(fakeCoreFramework);
    
    REQUIRE(performanceService->getMemoryWarningThreshold() == 0);
    
    performanceService->setMemoryWarningThreshold(500 * 1024 * 1024); // 500MB
    REQUIRE(performanceService->getMemoryWarningThreshold() == 500 * 1024 * 1024);
}

TEST_CASE("TimingTracker basic timing", "[PerformanceService][TimingTracker]")
{
    TimingTracker tracker;
    
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
    TimingTracker tracker;
    
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
    TimingTracker tracker;
    
    auto token1 = tracker.beginTiming("Op1");
    tracker.endTiming(token1);
    
    auto token2 = tracker.beginTiming("Op2");
    tracker.endTiming(token2);
    
    auto allStats = tracker.getAllStats();
    REQUIRE(allStats.size() == 2);
}

TEST_CASE("TimingTracker reset", "[PerformanceService][TimingTracker]")
{
    TimingTracker tracker;
    
    auto token = tracker.beginTiming("ToBeReset");
    tracker.endTiming(token);
    
    REQUIRE(tracker.getAllStats().size() == 1);
    
    tracker.reset();
    
    REQUIRE(tracker.getAllStats().empty());
}

TEST_CASE("PerformanceService snapshot", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = std::make_shared<PerformanceService>(fakeCoreFramework);
    
    // Add some timing data
    auto token = performanceService->beginTiming("SnapshotTest");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    performanceService->endTiming(token);
    
    auto snapshot = performanceService->takeSnapshot();
    
    REQUIRE(snapshot.memory.physicalBytes > 0);
    REQUIRE(snapshot.timingStats.size() == 1);
    REQUIRE(snapshot.timingStats[0].operationName == "SnapshotTest");
}

TEST_CASE("PerformanceService JSON export", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = std::make_shared<PerformanceService>(fakeCoreFramework);
    
    auto token = performanceService->beginTiming("JsonTest");
    performanceService->endTiming(token);
    
    auto json = performanceService->exportReportAsJson();
    
    REQUIRE(!json.empty());
    REQUIRE(json.find("timestamp") != std::string::npos);
    REQUIRE(json.find("memory") != std::string::npos);
    REQUIRE(json.find("JsonTest") != std::string::npos);
}

TEST_CASE("ScopedTiming RAII", "[PerformanceService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto performanceService = std::make_shared<PerformanceService>(fakeCoreFramework);
    
    {
        ScopedTiming timing(performanceService, "ScopedTest");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    auto stats = performanceService->getTimingStats("ScopedTest");
    REQUIRE(stats.callCount == 1);
    REQUIRE(stats.totalTime.count() >= 5);
}
