#pragma once

#include "../IMemoryMonitor.h"

#if defined(__APPLE__)

namespace ucf::service {

class MacOSMemoryMonitor : public IMemoryMonitor
{
public:
    MacOSMemoryMonitor() = default;
    ~MacOSMemoryMonitor() override = default;
    
    [[nodiscard]] MemoryInfo getMemoryUsage() const override;
};

} // namespace ucf::service

#endif // defined(__APPLE__)
