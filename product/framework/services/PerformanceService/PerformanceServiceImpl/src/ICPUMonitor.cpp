#include "ICPUMonitor.h"

#if defined(__APPLE__)
#include "platform/MacOSCPUMonitor.h"
#elif defined(__linux__)
#include "platform/LinuxCPUMonitor.h"
#elif defined(_WIN32)
#include "platform/WindowsCPUMonitor.h"
#endif

namespace ucf::service {

std::unique_ptr<ICPUMonitor> ICPUMonitor::create()
{
#if defined(__APPLE__)
    return std::make_unique<MacOSCPUMonitor>();
#elif defined(__linux__)
    return std::make_unique<LinuxCPUMonitor>();
#elif defined(_WIN32)
    return std::make_unique<WindowsCPUMonitor>();
#else
    return nullptr;
#endif
}

} // namespace ucf::service
