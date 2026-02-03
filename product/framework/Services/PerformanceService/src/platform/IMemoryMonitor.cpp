#include "IMemoryMonitor.h"

#if defined(__APPLE__)
#include "MacOSMemoryMonitor.h"
#elif defined(__linux__)
#include "LinuxMemoryMonitor.h"
#elif defined(_WIN32)
#include "WindowsMemoryMonitor.h"
#endif

namespace ucf::service {

std::unique_ptr<IMemoryMonitor> IMemoryMonitor::create()
{
#if defined(__APPLE__)
    return std::make_unique<MacOSMemoryMonitor>();
#elif defined(__linux__)
    return std::make_unique<LinuxMemoryMonitor>();
#elif defined(_WIN32)
    return std::make_unique<WindowsMemoryMonitor>();
#else
    return nullptr;
#endif
}

} // namespace ucf::service
