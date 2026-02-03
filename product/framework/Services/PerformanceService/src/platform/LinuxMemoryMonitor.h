#pragma once

#include "../IMemoryMonitor.h"

#if defined(__linux__)

namespace ucf::service {

class LinuxMemoryMonitor : public IMemoryMonitor
{
public:
    LinuxMemoryMonitor() = default;
    ~LinuxMemoryMonitor() override = default;
    
    [[nodiscard]] MemoryInfo getMemoryUsage() const override;
};

} // namespace ucf::service

#endif // defined(__linux__)
