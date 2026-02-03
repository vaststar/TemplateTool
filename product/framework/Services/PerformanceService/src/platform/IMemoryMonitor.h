#pragma once

#include <ucf/Services/PerformanceService/PerformanceInfo.h>

#include <memory>

namespace ucf::service {

/// Abstract interface for platform-specific memory monitoring
class IMemoryMonitor
{
public:
    virtual ~IMemoryMonitor() = default;
    
    /// Get current memory usage
    [[nodiscard]] virtual MemoryInfo getMemoryUsage() const = 0;
    
    /// Factory method
    [[nodiscard]] static std::unique_ptr<IMemoryMonitor> create();

protected:
    IMemoryMonitor() = default;
};

} // namespace ucf::service
