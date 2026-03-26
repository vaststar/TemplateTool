#include "ICPUMonitor.h"

#if defined(__APPLE__)
#include "platform/MacOSCPUMonitor.h"
#elif defined(__linux__)
#include "platform/LinuxCPUMonitor.h"
#elif defined(_WIN32)
#include "platform/WindowsCPUMonitor.h"
#endif

namespace ucf::service {

std::unique_ptr<ICPUMonitor> ICPUMonitor::create(std::chrono::milliseconds sampleInterval)
{
#if defined(__APPLE__)
    return std::make_unique<MacOSCPUMonitor>(sampleInterval);
#elif defined(__linux__)
    return std::make_unique<LinuxCPUMonitor>(sampleInterval);
#elif defined(_WIN32)
    return std::make_unique<WindowsCPUMonitor>(sampleInterval);
#else
    return nullptr;
#endif
}

} // namespace ucf::service
