#pragma once

#include <memory>
#include <functional>

namespace ucf::service {

struct MemoryInfo;

/// Callback interface for performance events
class IPerformanceServiceCallback
{
public:
    virtual ~IPerformanceServiceCallback() = default;

    /// Called when memory usage exceeds the warning threshold
    virtual void onMemoryWarning(const MemoryInfo& memoryInfo) {}

    /// Called when CPU usage exceeds the warning threshold.
    /// Default empty implementation so existing implementers are not forced to change.
    virtual void onCpuWarning(double /*cpuPercent*/) {}
};

using IPerformanceServiceCallbackPtr = std::shared_ptr<IPerformanceServiceCallback>;
using IPerformanceServiceCallbackWPtr = std::weak_ptr<IPerformanceServiceCallback>;

} // namespace ucf::service
