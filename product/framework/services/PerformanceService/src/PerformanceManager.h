#pragma once

#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <filesystem>

#include <ucf/Utilities/SinkNotifier/SinkNotifier.h>

#include <ucf/Services/PerformanceService/PerformanceInfo.h>

#include "PerformanceNotificationSink.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class IMemoryMonitor;
class ICPUMonitor;
class TimingTracker;

class PerformanceManager final : public ucf::utilities::SinkNotifier<IPerformanceNotificationSink>
{
public:
    explicit PerformanceManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~PerformanceManager();

    PerformanceManager(const PerformanceManager&) = delete;
    PerformanceManager(PerformanceManager&&) = delete;
    PerformanceManager& operator=(const PerformanceManager&) = delete;
    PerformanceManager& operator=(PerformanceManager&&) = delete;

public:
    void initialize();

public:
    // Memory Monitoring
    [[nodiscard]] MemoryInfo getCurrentMemoryUsage() const;
    void setMemoryWarningThreshold(uint64_t bytes);
    [[nodiscard]] uint64_t getMemoryWarningThreshold() const;

    // CPU Monitoring
    [[nodiscard]] double getCPUUsage() const;
    void setCpuWarningThreshold(double percent);
    [[nodiscard]] double getCpuWarningThreshold() const;

    // Timing
    [[nodiscard]] TimingToken beginTiming(const std::string& operationName);
    void endTiming(const TimingToken& token);
    [[nodiscard]] TimingStats getTimingStats(const std::string& operationName) const;
    [[nodiscard]] std::vector<TimingStats> getAllTimingStats() const;
    void resetTimingStats();

    // Snapshot & Export
    [[nodiscard]] PerformanceSnapshot takeSnapshot() const;
    [[nodiscard]] std::string exportReportAsJson() const;
    void exportReportToFile(const std::filesystem::path& path) const;

private:
    void startMonitoring();
    void stopMonitoring();
    void monitorLoop();

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::unique_ptr<IMemoryMonitor> mMemoryMonitor;
    std::unique_ptr<ICPUMonitor> mCPUMonitor;
    std::unique_ptr<TimingTracker> mTimingTracker;

    std::atomic<uint64_t> mMemoryWarningThreshold{0};  // 0 = disabled
    std::atomic<double> mCpuWarningThreshold{0.0};      // 0 or negative = disabled
    std::atomic<double> mCpuUsage{0.0};                 // last computed process CPU usage %
    std::atomic<double> mSystemCpuUsage{0.0};           // last computed system-wide CPU usage %

    std::chrono::milliseconds mSampleInterval{1000}; //ms between samples in monitoring loop
    std::chrono::milliseconds mReportInterval{30000};  // ms periodic usage log cadence
    std::atomic<bool> mMonitorRunning{false};
    std::thread mMonitorThread;
};

} // namespace ucf::service
