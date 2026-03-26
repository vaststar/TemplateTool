#pragma once

#include "../IMemoryMonitor.h"

#if defined(_WIN32)

namespace ucf::service {

class WindowsMemoryMonitor : public IMemoryMonitor
{
public:
    WindowsMemoryMonitor() = default;
    ~WindowsMemoryMonitor() override = default;
    
    [[nodiscard]] MemoryInfo getMemoryUsage() const override;
};

} // namespace ucf::service

#endif // defined(_WIN32)
