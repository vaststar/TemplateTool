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
    virtual void onMemoryWarning(const MemoryInfo& memoryInfo) = 0;
};

using IPerformanceServiceCallbackPtr = std::shared_ptr<IPerformanceServiceCallback>;
using IPerformanceServiceCallbackWPtr = std::weak_ptr<IPerformanceServiceCallback>;

} // namespace ucf::service
