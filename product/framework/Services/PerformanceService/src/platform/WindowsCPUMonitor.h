#pragma once

#include "../ICPUMonitor.h"
#include <atomic>
#include <thread>
#include <chrono>

namespace ucf::service {

class WindowsCPUMonitor : public ICPUMonitor
{
public:
    explicit WindowsCPUMonitor(std::chrono::milliseconds sampleInterval);
    ~WindowsCPUMonitor() override;
    
    void start() override;
    void stop() override;
    [[nodiscard]] double getCPUUsage() const override;
    [[nodiscard]] unsigned int getCPUCoreCount() const override;

private:
    void sampleLoop();
    
    /// Get process CPU time in 100-nanosecond intervals
    [[nodiscard]] uint64_t getProcessCPUTime() const;

private:
    std::chrono::milliseconds mSampleInterval;
    std::atomic<double> mCPUUsage{0.0};
    std::atomic<bool> mRunning{false};
    std::thread mSampleThread;
};

} // namespace ucf::service
