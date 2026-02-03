#pragma once

#include "ICPUMonitor.h"
#include <atomic>
#include <thread>
#include <chrono>

namespace ucf::service {

class MacOSCPUMonitor : public ICPUMonitor
{
public:
    explicit MacOSCPUMonitor(std::chrono::milliseconds sampleInterval);
    ~MacOSCPUMonitor() override;
    
    void start() override;
    void stop() override;
    [[nodiscard]] double getCPUUsage() const override;
    [[nodiscard]] unsigned int getCPUCoreCount() const override;

private:
    void sampleLoop();
    
    /// Get process CPU time in microseconds
    [[nodiscard]] uint64_t getProcessCPUTime() const;

private:
    std::chrono::milliseconds mSampleInterval;
    std::atomic<double> mCPUUsage{0.0};
    std::atomic<bool> mRunning{false};
    std::thread mSampleThread;
};

} // namespace ucf::service
