#pragma once

#include "../ICPUMonitor.h"
#include <atomic>
#include <thread>
#include <chrono>

namespace ucf::service {

class LinuxCPUMonitor : public ICPUMonitor
{
public:
    explicit LinuxCPUMonitor(std::chrono::milliseconds sampleInterval);
    ~LinuxCPUMonitor() override;
    
    void start() override;
    void stop() override;
    [[nodiscard]] double getCPUUsage() const override;
    [[nodiscard]] unsigned int getCPUCoreCount() const override;

private:
    void sampleLoop();
    
    /// Get process CPU time in clock ticks
    [[nodiscard]] uint64_t getProcessCPUTime() const;

private:
    std::chrono::milliseconds mSampleInterval;
    std::atomic<double> mCPUUsage{0.0};
    std::atomic<bool> mRunning{false};
    std::thread mSampleThread;
    long mClockTicksPerSecond{100};
};

} // namespace ucf::service
