#include "PerformanceManager.h"
#include "PerformanceServiceLogger.h"
#include "TimingTracker.h"
#include "IMemoryMonitor.h"
#include "ICPUMonitor.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Utilities/JsonUtils/JsonBuilder.h>

#include <fstream>
#include <ctime>

namespace ucf::service {

PerformanceManager::PerformanceManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mMemoryMonitor(IMemoryMonitor::create())
    , mCPUMonitor(ICPUMonitor::create())
    , mTimingTracker(std::make_unique<TimingTracker>())
{
    PERFORMANCE_LOG_DEBUG("PerformanceManager created");
}

PerformanceManager::~PerformanceManager()
{
    stopCPUMonitoring();
    PERFORMANCE_LOG_DEBUG("PerformanceManager destroyed");
}

void PerformanceManager::initialize()
{
    startCPUMonitoring();
    PERFORMANCE_LOG_INFO("PerformanceManager initialized");
}

// ==========================================
// Memory Monitoring
// ==========================================

MemoryInfo PerformanceManager::getCurrentMemoryUsage() const
{
    if (mMemoryMonitor)
    {
        return mMemoryMonitor->getMemoryUsage();
    }
    return MemoryInfo{};
}

void PerformanceManager::setMemoryWarningThreshold(uint64_t bytes)
{
    mMemoryWarningThreshold.store(bytes);
    PERFORMANCE_LOG_INFO("Memory warning threshold set to " << bytes / 1024 / 1024 << " MB");
}

uint64_t PerformanceManager::getMemoryWarningThreshold() const
{
    return mMemoryWarningThreshold.load();
}

// ==========================================
// CPU Monitoring
// ==========================================

double PerformanceManager::getCPUUsage() const
{
    if (mCPUMonitor)
    {
        return mCPUMonitor->getCPUUsage();
    }
    return 0.0;
}

void PerformanceManager::startCPUMonitoring()
{
    if (mCPUMonitor)
    {
        mCPUMonitor->start();
        PERFORMANCE_LOG_INFO("CPU monitoring started");
    }
}

void PerformanceManager::stopCPUMonitoring()
{
    if (mCPUMonitor)
    {
        mCPUMonitor->stop();
        PERFORMANCE_LOG_INFO("CPU monitoring stopped");
    }
}

// ==========================================
// Timing
// ==========================================

TimingToken PerformanceManager::beginTiming(const std::string& operationName)
{
    return mTimingTracker->beginTiming(operationName);
}

void PerformanceManager::endTiming(const TimingToken& token)
{
    mTimingTracker->endTiming(token);
}

TimingStats PerformanceManager::getTimingStats(const std::string& operationName) const
{
    return mTimingTracker->getStats(operationName);
}

std::vector<TimingStats> PerformanceManager::getAllTimingStats() const
{
    return mTimingTracker->getAllStats();
}

void PerformanceManager::resetTimingStats()
{
    mTimingTracker->reset();
}

// ==========================================
// Snapshot & Export
// ==========================================

PerformanceSnapshot PerformanceManager::takeSnapshot() const
{
    PerformanceSnapshot snapshot;
    snapshot.timestamp = std::chrono::system_clock::now();
    snapshot.memory = getCurrentMemoryUsage();
    snapshot.cpuUsagePercent = getCPUUsage();
    snapshot.timingStats = getAllTimingStats();
    return snapshot;
}

std::string PerformanceManager::exportReportAsJson() const
{
    auto snapshot = takeSnapshot();
    
    // Timestamp
    auto time_t = std::chrono::system_clock::to_time_t(snapshot.timestamp);
    std::tm tm_buf{};
#ifdef _WIN32
    gmtime_s(&tm_buf, &time_t);
#else
    gmtime_r(&time_t, &tm_buf);
#endif
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%dT%H:%M:%SZ", &tm_buf);
    
    // Memory
    utilities::JsonBuilder memory;
    memory.add("physicalMB", snapshot.memory.physicalBytes / 1024 / 1024)
          .add("virtualMB", snapshot.memory.virtualBytes / 1024 / 1024)
          .add("peakPhysicalMB", snapshot.memory.peakPhysicalBytes / 1024 / 1024)
          .add("availableSystemMB", snapshot.memory.availableSystemBytes / 1024 / 1024);
    
    // Timing stats
    std::vector<utilities::JsonBuilder> timingArray;
    for (const auto& stats : snapshot.timingStats)
    {
        utilities::JsonBuilder timing;
        timing.add("operation", stats.operationName)
              .add("callCount", static_cast<uint64_t>(stats.callCount))
              .add("totalMs", static_cast<int64_t>(stats.totalTime.count()))
              .add("avgMs", static_cast<int64_t>(stats.avgTime().count()))
              .add("minMs", static_cast<int64_t>(stats.minTime == std::chrono::milliseconds::max() ? 0 : stats.minTime.count()))
              .add("maxMs", static_cast<int64_t>(stats.maxTime.count()));
        timingArray.push_back(std::move(timing));
    }
    
    // Build report
    utilities::JsonBuilder report;
    report.add("timestamp", timeBuf)
          .addObject("memory", memory)
          .add("cpuUsagePercent", snapshot.cpuUsagePercent)
          .addArray("timingStats", timingArray);
    
    return report.buildPretty(2);
}

void PerformanceManager::exportReportToFile(const std::filesystem::path& path) const
{
    std::ofstream file(path);
    if (file.is_open())
    {
        file << exportReportAsJson();
        PERFORMANCE_LOG_INFO("Performance report exported to: " << path.string());
    }
    else
    {
        PERFORMANCE_LOG_ERROR("Failed to export performance report to: " << path.string());
    }
}

} // namespace ucf::service
