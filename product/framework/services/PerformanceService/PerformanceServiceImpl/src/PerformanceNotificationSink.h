#pragma once

namespace ucf::service {

struct MemoryInfo;

// Internal sink: PerformanceManager invokes it from the monitoring thread when a
// threshold is exceeded; PerformanceService translates it into outward callbacks.
class IPerformanceNotificationSink
{
public:
    virtual ~IPerformanceNotificationSink() = default;

    /// Memory physical usage exceeded the configured warning threshold.
    virtual void onMemoryWarning(const MemoryInfo& memoryInfo) = 0;

    /// Process CPU usage exceeded the configured warning threshold.
    virtual void onCpuWarning(double cpuPercent) = 0;
};

} // namespace ucf::service
