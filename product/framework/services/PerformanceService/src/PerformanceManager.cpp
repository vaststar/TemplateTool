#include "PerformanceManager.h"
#include "PerformanceServiceLogger.h"
#include "PerformanceNotificationSink.h"
#include "TimingTracker.h"
#include "IMemoryMonitor.h"
#include "ICPUMonitor.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Utilities/JsonUtils/JsonValue.h>

#include <algorithm>
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
    stopMonitoring();
    PERFORMANCE_LOG_DEBUG("PerformanceManager destroyed");
}

void PerformanceManager::initialize()
{
    startMonitoring();
    PERFORMANCE_LOG_INFO("PerformanceManager initialized");
}

void PerformanceManager::setNotificationSink(std::weak_ptr<IPerformanceNotificationSink> sink)
{
    mNotificationSink = std::move(sink);
    PERFORMANCE_LOG_DEBUG("PerformanceManager notification sink set");
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
    return mCpuUsage.load();
}

void PerformanceManager::setCpuWarningThreshold(double percent)
{
    mCpuWarningThreshold.store(percent);
    PERFORMANCE_LOG_INFO("CPU warning threshold set to " << percent << " %");
}

double PerformanceManager::getCpuWarningThreshold() const
{
    return mCpuWarningThreshold.load();
}

// ==========================================
// Monitoring loop
// ==========================================

void PerformanceManager::startMonitoring()
{
    if (mMonitorRunning.exchange(true))
    {
        return;
    }
    mMonitorThread = std::thread(&PerformanceManager::monitorLoop, this);
    PERFORMANCE_LOG_INFO("Performance monitoring started");
}

void PerformanceManager::stopMonitoring()
{
    if (!mMonitorRunning.exchange(false))
    {
        return;
    }
    if (mMonitorThread.joinable())
    {
        mMonitorThread.join();
    }
    PERFORMANCE_LOG_INFO("Performance monitoring stopped");
}

void PerformanceManager::monitorLoop()
{
    uint64_t prevCpuMicros = mCPUMonitor ? mCPUMonitor->getProcessCpuTimeMicros() : 0;
    SystemCpuTimes prevSysTimes = mCPUMonitor ? mCPUMonitor->getSystemCpuTimes() : SystemCpuTimes{};
    auto prevWallTime = std::chrono::steady_clock::now();

    // Core count is used to normalize process CPU to a whole-machine 0-100% scale,
    // so it is comparable to (and never exceeds) the system-wide CPU usage.
    const double coreCount = mCPUMonitor
        ? std::max(1u, mCPUMonitor->getCpuCoreCount())
        : 1u;

    // Periodic usage report cadence (e.g. every minute), expressed in sample ticks.
    const uint64_t reportEverySamples = (mSampleInterval.count() > 0)
        ? std::max<uint64_t>(1, mReportInterval.count() / mSampleInterval.count())
        : 1;
    uint64_t sampleCount = 0;
    double cpuSumInWindow = 0.0;      // accumulates per-sample process CPU usage within a report window
    double sysCpuSumInWindow = 0.0;   // accumulates per-sample system CPU usage within a report window

    while (mMonitorRunning.load())
    {
        std::this_thread::sleep_for(mSampleInterval);
        if (!mMonitorRunning.load())
        {
            break;
        }
        ++sampleCount;

        // --- CPU: compute usage from cumulative CPU time delta ---
        if (mCPUMonitor)
        {
            uint64_t currCpuMicros = mCPUMonitor->getProcessCpuTimeMicros();
            auto currWallTime = std::chrono::steady_clock::now();

            if (auto wallMicros = std::chrono::duration_cast<std::chrono::microseconds>(currWallTime - prevWallTime).count(); wallMicros > 0)
            {
                // Normalize by core count -> whole-machine percentage (0-100%).
                double usage = (static_cast<double>(currCpuMicros - prevCpuMicros) /
                                static_cast<double>(wallMicros)) * 100.0 / coreCount;
                mCpuUsage.store(usage);
                cpuSumInWindow += usage;

                double cpuThreshold = mCpuWarningThreshold.load();
                if (cpuThreshold > 0.0 && usage > cpuThreshold)
                {
                    PERFORMANCE_LOG_WARN("CPU warning: usage " << usage << "% exceeded threshold " << cpuThreshold << "%");
                    if (auto sink = mNotificationSink.lock())
                    {
                        sink->onCpuWarning(usage);
                    }
                }
            }

            // System-wide CPU usage from busy/total delta
            SystemCpuTimes currSysTimes = mCPUMonitor->getSystemCpuTimes();
            if (uint64_t totalDelta = currSysTimes.totalMicros - prevSysTimes.totalMicros; totalDelta > 0)
            {
                uint64_t busyDelta = currSysTimes.busyMicros - prevSysTimes.busyMicros;
                double sysUsage = (static_cast<double>(busyDelta) / static_cast<double>(totalDelta)) * 100.0;
                mSystemCpuUsage.store(sysUsage);
                sysCpuSumInWindow += sysUsage;
            }

            prevCpuMicros = currCpuMicros;
            prevSysTimes = currSysTimes;
            prevWallTime = currWallTime;
        }

        // --- Memory: compare physical usage against threshold ---
        if (uint64_t memThreshold = mMemoryWarningThreshold.load(); memThreshold > 0 && mMemoryMonitor)
        {
            MemoryInfo info = mMemoryMonitor->getMemoryUsage();
            if (info.physicalBytes > memThreshold)
            {
                PERFORMANCE_LOG_WARN("Memory warning: physical " << info.physicalBytes / 1024 / 1024
                    << " MB exceeded threshold " << memThreshold / 1024 / 1024 << " MB");
                if (auto sink = mNotificationSink.lock())
                {
                    sink->onMemoryWarning(info);
                }
            }
        }

        // --- Periodic usage report (CPU averaged over the window) ---
        if (sampleCount % reportEverySamples == 0)
        {
            double avgCpu = cpuSumInWindow / static_cast<double>(reportEverySamples);
            double avgSysCpu = sysCpuSumInWindow / static_cast<double>(reportEverySamples);
            cpuSumInWindow = 0.0;
            sysCpuSumInWindow = 0.0;

            MemoryInfo mem = mMemoryMonitor ? mMemoryMonitor->getMemoryUsage() : MemoryInfo{};
            PERFORMANCE_LOG_INFO("Performance usage: "
                << "CPU{process: avg " << avgCpu << "%, last " << mCpuUsage.load() << "%; "
                << "system: avg " << avgSysCpu << "%, last " << mSystemCpuUsage.load() << "%} "
                << "MEM{process[physical " << mem.physicalBytes / 1024 / 1024 << " MB, "
                << "private " << mem.virtualBytes / 1024 / 1024 << " MB, "
                << "peak " << mem.peakPhysicalBytes / 1024 / 1024 << " MB], "
                << "system[avail " << mem.availableSystemBytes / 1024 / 1024 << " MB]}");
        }
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
    utilities::JsonValue memory = utilities::JsonValue::object();
    memory.set("physicalMB", utilities::JsonValue(static_cast<uint64_t>(snapshot.memory.physicalBytes / 1024 / 1024)));
    memory.set("virtualMB", utilities::JsonValue(static_cast<uint64_t>(snapshot.memory.virtualBytes / 1024 / 1024)));
    memory.set("peakPhysicalMB", utilities::JsonValue(static_cast<uint64_t>(snapshot.memory.peakPhysicalBytes / 1024 / 1024)));
    memory.set("availableSystemMB", utilities::JsonValue(static_cast<uint64_t>(snapshot.memory.availableSystemBytes / 1024 / 1024)));

    // Timing stats
    utilities::JsonValue timingArray = utilities::JsonValue::array();
    for (const auto& stats : snapshot.timingStats)
    {
        utilities::JsonValue timing = utilities::JsonValue::object();
        timing.set("operation", utilities::JsonValue(stats.operationName));
        timing.set("callCount", utilities::JsonValue(static_cast<uint64_t>(stats.callCount)));
        timing.set("totalMs", utilities::JsonValue(static_cast<int64_t>(stats.totalTime.count())));
        timing.set("avgMs", utilities::JsonValue(static_cast<int64_t>(stats.avgTime().count())));
        timing.set("minMs", utilities::JsonValue(static_cast<int64_t>(stats.minTime == std::chrono::milliseconds::max() ? 0 : stats.minTime.count())));
        timing.set("maxMs", utilities::JsonValue(static_cast<int64_t>(stats.maxTime.count())));
        timingArray.push_back(std::move(timing));
    }

    // Build report
    utilities::JsonValue report = utilities::JsonValue::object();
    report.set("timestamp", utilities::JsonValue(timeBuf));
    report.set("memory", std::move(memory));
    report.set("cpuUsagePercent", utilities::JsonValue(snapshot.cpuUsagePercent));
    report.set("timingStats", std::move(timingArray));

    return report.dumpPretty(2);
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
