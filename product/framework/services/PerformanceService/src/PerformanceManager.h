#pragma once

#include <ucf/Services/PerformanceService/PerformanceInfo.h>

#include <memory>
#include <atomic>
#include <filesystem>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class IMemoryMonitor;
class ICPUMonitor;
class TimingTracker;

class PerformanceManager final
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
    void startCPUMonitoring();
    void stopCPUMonitoring();

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
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::unique_ptr<IMemoryMonitor> mMemoryMonitor;
    std::unique_ptr<ICPUMonitor> mCPUMonitor;
    std::unique_ptr<TimingTracker> mTimingTracker;
    
    std::atomic<uint64_t> mMemoryWarningThreshold{0};  // 0 = disabled
};

} // namespace ucf::service
