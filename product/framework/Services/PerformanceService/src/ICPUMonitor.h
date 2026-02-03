#pragma once

#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

namespace ucf::service {

/// Interface for CPU monitoring with background sampling
class ICPUMonitor
{
public:
    virtual ~ICPUMonitor() = default;
    
    /// Start background sampling thread
    virtual void start() = 0;
    
    /// Stop background sampling thread
    virtual void stop() = 0;
    
    /// Get current CPU usage percentage (0-100)
    /// Returns the value from the last sampling interval
    [[nodiscard]] virtual double getCPUUsage() const = 0;
    
    /// Get the number of CPU cores
    [[nodiscard]] virtual unsigned int getCPUCoreCount() const = 0;
    
    /// Factory method
    static std::unique_ptr<ICPUMonitor> create(std::chrono::milliseconds sampleInterval = std::chrono::milliseconds(1000));
};

} // namespace ucf::service
