#include "IMemoryMonitor.h"

#if defined(__APPLE__)
#include "platform/MacOSMemoryMonitor.h"
#elif defined(__linux__)
#include "platform/LinuxMemoryMonitor.h"
#elif defined(_WIN32)
#include "platform/WindowsMemoryMonitor.h"
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
