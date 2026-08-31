#include "PerformanceManager.h"
#include "PerformanceServiceLogger.h"
#include "PerformanceCpuCalculator.h"
#include "PerformanceMonitoringSchedule.h"
#include "TimingTracker.h"
#include "IMemoryMonitor.h"
#include "ICPUMonitor.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/utilities/JsonUtils/JsonValue.h>
#include <ucf/utilities/TimeUtils/TimeUtils.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace ucf::service {
namespace {

std::string formatPercentage(std::optional<double> percentage)
{
    if (!percentage)
    {
        return "unavailable";
    }

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << *percentage << '%';
    return stream.str();
}

std::string formatMemorySize(std::optional<uint64_t> bytes)
{
    if (!bytes)
    {
        return "unavailable";
    }

    constexpr double bytesPerMebibyte = 1024.0 * 1024.0;
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2)
           << static_cast<double>(*bytes) / bytesPerMebibyte << " MiB";
    return stream.str();
}

utilities::JsonValue optionalByteCountToJson(std::optional<uint64_t> bytes)
{
    return bytes ? utilities::JsonValue(*bytes) : utilities::JsonValue(nullptr);
}

utilities::JsonValue optionalTimingDurationToJson(
    std::optional<TimingDuration> duration)
{
    return duration
        ? utilities::JsonValue(static_cast<int64_t>(duration->count()))
        : utilities::JsonValue(nullptr);
}

} // namespace

PerformanceManager::PerformanceManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mMemoryMonitor(IMemoryMonitor::create())
    , mCPUMonitor(ICPUMonitor::create())
    , mTimingTracker(std::make_unique<TimingTracker>())
{
    PERFORMANCE_LOG_DEBUG("PerformanceManager constructing, address: " << this);
    PERFORMANCE_LOG_DEBUG("PerformanceManager constructed, address: " << this);
}

PerformanceManager::~PerformanceManager()
{
    PERFORMANCE_LOG_DEBUG("PerformanceManager destroying, address: " << this);
    stopMonitoring();
    PERFORMANCE_LOG_DEBUG("PerformanceManager destructor body finished, address: " << this);
}

void PerformanceManager::initialize()
{
    PERFORMANCE_LOG_INFO("PerformanceManager initialization started, address: " << this);

    startMonitoring();

    PERFORMANCE_LOG_INFO("PerformanceManager initialization finished, address: " << this);
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

void PerformanceManager::setProcessResidentMemoryWarningThreshold(uint64_t bytes)
{
    mProcessResidentMemoryWarningThreshold.store(bytes);
    PERFORMANCE_LOG_INFO(
        "Process resident-memory warning threshold set to "
        << formatMemorySize(bytes) << (bytes == 0 ? " (disabled)" : ""));
}

uint64_t PerformanceManager::getProcessResidentMemoryWarningThreshold() const
{
    return mProcessResidentMemoryWarningThreshold.load();
}

// ==========================================
// CPU Monitoring
// ==========================================

std::optional<double> PerformanceManager::getProcessCpuUsagePercent() const
{
    if (!mProcessCpuUsageAvailable.load())
    {
        return std::nullopt;
    }
    return mProcessCpuUsagePercent.load();
}

std::optional<double> PerformanceManager::getSystemCpuUsagePercent() const
{
    if (!mSystemCpuUsageAvailable.load())
    {
        return std::nullopt;
    }
    return mSystemCpuUsagePercent.load();
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
    PERFORMANCE_LOG_INFO("PerformanceManager monitoring startup started, address: " << this);

    if (mMonitorRunning.exchange(true))
    {
        PERFORMANCE_LOG_DEBUG(
            "PerformanceManager monitoring startup skipped: monitoring is already running, address: "
            << this);
        return;
    }
    mMonitorThread = std::thread(&PerformanceManager::monitorLoop, this);
    PERFORMANCE_LOG_INFO("PerformanceManager monitoring startup finished, address: " << this);
}

void PerformanceManager::stopMonitoring()
{
    PERFORMANCE_LOG_INFO("PerformanceManager monitoring shutdown started, address: " << this);

    if (!mMonitorRunning.exchange(false))
    {
        PERFORMANCE_LOG_DEBUG(
            "PerformanceManager monitoring shutdown skipped: monitoring is not running, address: "
            << this);
        return;
    }
    mMonitorCv.notify_all();
    if (mMonitorThread.joinable())
    {
        mMonitorThread.join();
    }
    PERFORMANCE_LOG_INFO("PerformanceManager monitoring shutdown finished, address: " << this);
}

void PerformanceManager::monitorLoop()
{
    PERFORMANCE_LOG_DEBUG("PerformanceManager monitoring loop started, address: " << this);

    PerformanceCpuCalculator cpuCalculator;
    CpuUsageWindow processCpuWindow;
    CpuUsageWindow systemCpuWindow;

    // Establish independent process and system baselines before the first wait.
    const auto initialProcessCpuTime = mCPUMonitor
        ? mCPUMonitor->getProcessCpuTime()
        : std::nullopt;
    const auto initialSystemCpuTimes = mCPUMonitor
        ? mCPUMonitor->getSystemCpuTimes()
        : std::nullopt;
    const auto scheduleStartTime = PerformanceMonitoringSchedule::Clock::now();
    static_cast<void>(cpuCalculator.update(
        initialProcessCpuTime,
        initialSystemCpuTimes,
        scheduleStartTime));

    PerformanceMonitoringSchedule schedule(
        scheduleStartTime, mSampleInterval, mReportInterval);
    bool processSamplingFailureReported = false;
    bool systemSamplingFailureReported = false;
    bool processResidentMemorySamplingFailureReported = false;

    while (mMonitorRunning.load())
    {
        {
            // Wait for the absolute sample deadline, but wake immediately when stopped.
            std::unique_lock<std::mutex> lock(mMonitorMutex);
            mMonitorCv.wait_until(
                lock,
                schedule.nextSampleDeadline(),
                [this] { return !mMonitorRunning.load(); });
        }
        if (!mMonitorRunning.load())
        {
            break;
        }

        // Keep the original cadence. If execution was delayed, skip all elapsed
        // slots and schedule the next sample strictly in the future.
        schedule.advanceSampleDeadline(PerformanceMonitoringSchedule::Clock::now());

        // --- CPU: compute usage from monotonic cumulative counter deltas ---
        if (mCPUMonitor)
        {
            const CpuUsageSample cpuSample = cpuCalculator.update(
                mCPUMonitor->getProcessCpuTime(),
                mCPUMonitor->getSystemCpuTimes(),
                std::chrono::steady_clock::now());

            if (cpuSample.process.status == CpuSampleStatus::Valid)
            {
                const double usage = *cpuSample.process.percent;
                mProcessCpuUsagePercent.store(usage);
                mProcessCpuUsageAvailable.store(true);
                processCpuWindow.add(usage);

                if (processSamplingFailureReported)
                {
                    PERFORMANCE_LOG_INFO("Process CPU sampling recovered");
                    processSamplingFailureReported = false;
                }

                const double cpuThreshold = mCpuWarningThreshold.load();
                if (cpuThreshold > 0.0 && usage > cpuThreshold)
                {
                    PERFORMANCE_LOG_WARN("CPU warning: usage " << usage << "% exceeded threshold " << cpuThreshold << "%");
                    notifySink(&IPerformanceNotificationSink::onCpuWarning, usage);
                }
            }
            else
            {
                mProcessCpuUsageAvailable.store(false);
                if (cpuSample.process.status == CpuSampleStatus::CounterReset)
                {
                    PERFORMANCE_LOG_WARN(
                        "Process CPU counter moved backwards; baseline reset, previous: "
                        << cpuSample.process.previousTime.count()
                        << " us, current: " << cpuSample.process.currentTime.count() << " us");
                }
                else if ((cpuSample.process.status == CpuSampleStatus::Unavailable ||
                          cpuSample.process.status == CpuSampleStatus::Invalid) &&
                         !processSamplingFailureReported)
                {
                    PERFORMANCE_LOG_WARN("Process CPU sampling became unavailable");
                    processSamplingFailureReported = true;
                }
            }

            if (cpuSample.system.status == CpuSampleStatus::Valid)
            {
                const double usage = *cpuSample.system.percent;
                mSystemCpuUsagePercent.store(usage);
                mSystemCpuUsageAvailable.store(true);
                systemCpuWindow.add(usage);

                if (systemSamplingFailureReported)
                {
                    PERFORMANCE_LOG_INFO("System CPU sampling recovered");
                    systemSamplingFailureReported = false;
                }
            }
            else
            {
                mSystemCpuUsageAvailable.store(false);
                if (cpuSample.system.status == CpuSampleStatus::CounterReset)
                {
                    PERFORMANCE_LOG_WARN(
                        "System CPU counters moved backwards; baseline reset, previous: {busy: "
                        << cpuSample.system.previousTimes.busyTicks
                        << ", total: " << cpuSample.system.previousTimes.totalTicks
                        << "}, current: {busy: " << cpuSample.system.currentTimes.busyTicks
                        << ", total: " << cpuSample.system.currentTimes.totalTicks << '}');
                }
                else if ((cpuSample.system.status == CpuSampleStatus::Unavailable ||
                          cpuSample.system.status == CpuSampleStatus::Invalid) &&
                         !systemSamplingFailureReported)
                {
                    PERFORMANCE_LOG_WARN("System CPU sampling became unavailable");
                    systemSamplingFailureReported = true;
                }
            }
        }
        else
        {
            mProcessCpuUsageAvailable.store(false);
            mSystemCpuUsageAvailable.store(false);
            if (!processSamplingFailureReported)
            {
                PERFORMANCE_LOG_WARN("Process CPU sampling became unavailable: monitor is not available");
                processSamplingFailureReported = true;
            }
            if (!systemSamplingFailureReported)
            {
                PERFORMANCE_LOG_WARN("System CPU sampling became unavailable: monitor is not available");
                systemSamplingFailureReported = true;
            }
        }

        // --- Memory: compare process resident memory against its threshold ---
        if (const uint64_t memoryThreshold = mProcessResidentMemoryWarningThreshold.load();
            memoryThreshold > 0)
        {
            const MemoryInfo info = mMemoryMonitor
                ? mMemoryMonitor->getMemoryUsage()
                : MemoryInfo{};
            if (info.processResidentBytes)
            {
                if (processResidentMemorySamplingFailureReported)
                {
                    PERFORMANCE_LOG_INFO("Process resident-memory sampling recovered");
                    processResidentMemorySamplingFailureReported = false;
                }

                if (*info.processResidentBytes > memoryThreshold)
                {
                    PERFORMANCE_LOG_WARN(
                        "Process resident-memory warning: usage "
                        << formatMemorySize(info.processResidentBytes)
                        << " exceeded threshold " << formatMemorySize(memoryThreshold));
                    notifySink(&IPerformanceNotificationSink::onMemoryWarning, info);
                }
            }
            else if (!processResidentMemorySamplingFailureReported)
            {
                PERFORMANCE_LOG_WARN("Process resident-memory sampling became unavailable");
                processResidentMemorySamplingFailureReported = true;
            }
        }

        // --- Periodic usage report (CPU averaged over the window) ---
        if (schedule.consumeReportDeadline(PerformanceMonitoringSchedule::Clock::now()))
        {
            const auto avgProcessCpu = processCpuWindow.average();
            const auto avgSystemCpu = systemCpuWindow.average();

            const MemoryInfo mem = mMemoryMonitor
                ? mMemoryMonitor->getMemoryUsage()
                : MemoryInfo{};
            PERFORMANCE_LOG_INFO("Performance usage: "
                << "CPU{process: avg " << formatPercentage(avgProcessCpu)
                << ", last " << formatPercentage(getProcessCpuUsagePercent()) << "; "
                << "system: avg " << formatPercentage(avgSystemCpu)
                << ", last " << formatPercentage(getSystemCpuUsagePercent()) << "} "
                << "MEM{process[resident " << formatMemorySize(mem.processResidentBytes)
                << ", peak resident " << formatMemorySize(mem.processPeakResidentBytes)
                << ", virtual address space " << formatMemorySize(mem.processVirtualAddressSpaceBytes)
                << ", private committed " << formatMemorySize(mem.processPrivateCommittedBytes)
                << "], system[available physical "
                << formatMemorySize(mem.systemAvailablePhysicalBytes) << "]}");

            processCpuWindow.reset();
            systemCpuWindow.reset();
        }
    }

    PERFORMANCE_LOG_DEBUG("PerformanceManager monitoring loop finished, address: " << this);
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

std::optional<TimingStats> PerformanceManager::getTimingStats(
    const std::string& operationName) const
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
    snapshot.processCpuUsagePercent = getProcessCpuUsagePercent();
    snapshot.systemCpuUsagePercent = getSystemCpuUsagePercent();
    snapshot.timingStats = getAllTimingStats();
    return snapshot;
}

std::string PerformanceManager::exportReportAsJson() const
{
    auto snapshot = takeSnapshot();

    // Timestamp
    const auto unixMilliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            snapshot.timestamp.time_since_epoch())
            .count();
    const auto timestamp = utilities::TimeUtils::format(
        utilities::Instant::fromUnixMilliseconds(unixMilliseconds),
        utilities::TimeZone::utc(),
        "%Y-%m-%dT%H:%M:%SZ");

    // Memory
    utilities::JsonValue memory = utilities::JsonValue::object();
    memory.set(
        "processResidentBytes",
        optionalByteCountToJson(snapshot.memory.processResidentBytes));
    memory.set(
        "processPeakResidentBytes",
        optionalByteCountToJson(snapshot.memory.processPeakResidentBytes));
    memory.set(
        "processVirtualAddressSpaceBytes",
        optionalByteCountToJson(snapshot.memory.processVirtualAddressSpaceBytes));
    memory.set(
        "processPrivateCommittedBytes",
        optionalByteCountToJson(snapshot.memory.processPrivateCommittedBytes));
    memory.set(
        "systemAvailablePhysicalBytes",
        optionalByteCountToJson(snapshot.memory.systemAvailablePhysicalBytes));

    // Timing stats
    utilities::JsonValue timingArray = utilities::JsonValue::array();
    for (const auto& stats : snapshot.timingStats)
    {
        utilities::JsonValue timing = utilities::JsonValue::object();
        timing.set("operation", utilities::JsonValue(stats.operationName));
        timing.set("callCount", utilities::JsonValue(static_cast<uint64_t>(stats.callCount)));
        timing.set(
            "totalDurationMicroseconds",
            utilities::JsonValue(static_cast<int64_t>(stats.totalDuration.count())));
        timing.set(
            "averageDurationMicroseconds",
            optionalTimingDurationToJson(stats.averageDuration()));
        timing.set(
            "minimumDurationMicroseconds",
            optionalTimingDurationToJson(stats.minimumDuration));
        timing.set(
            "maximumDurationMicroseconds",
            optionalTimingDurationToJson(stats.maximumDuration));
        timingArray.push_back(std::move(timing));
    }

    // Build report
    utilities::JsonValue report = utilities::JsonValue::object();
    if (timestamp)
    {
        report.set("timestamp", utilities::JsonValue(timestamp.value()));
    }
    else
    {
        PERFORMANCE_LOG_WARN(
            "Failed to format performance report timestamp, code: "
            << static_cast<int>(timestamp.error().code)
            << ", reason: " << timestamp.error().diagnostic);
        report.set("timestamp", utilities::JsonValue(nullptr));
    }
    report.set("memory", std::move(memory));
    report.set(
        "processCpuUsagePercent",
        snapshot.processCpuUsagePercent
            ? utilities::JsonValue(*snapshot.processCpuUsagePercent)
            : utilities::JsonValue(nullptr));
    report.set(
        "systemCpuUsagePercent",
        snapshot.systemCpuUsagePercent
            ? utilities::JsonValue(*snapshot.systemCpuUsagePercent)
            : utilities::JsonValue(nullptr));
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
