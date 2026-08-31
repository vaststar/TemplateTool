#include "WindowsCPUMonitor.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace ucf::service {

std::optional<ProcessCpuTime> WindowsCPUMonitor::getProcessCpuTime() const
{
#ifdef _WIN32
    FILETIME createTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime))
    {
        // FILETIME is in 100-nanosecond intervals
        ULARGE_INTEGER kernel, user;
        kernel.LowPart = kernelTime.dwLowDateTime;
        kernel.HighPart = kernelTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;
        user.HighPart = userTime.dwHighDateTime;

        // Convert 100-ns intervals to microseconds
        return ProcessCpuTime{(kernel.QuadPart + user.QuadPart) / 10};
    }
#endif
    return std::nullopt;
}

std::optional<SystemCpuTimes> WindowsCPUMonitor::getSystemCpuTimes() const
{
#ifdef _WIN32
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime))
    {
        // All FILETIME are in 100-ns intervals. kernelTime already includes idleTime.
        ULARGE_INTEGER idle, kernel, user;
        idle.LowPart = idleTime.dwLowDateTime;
        idle.HighPart = idleTime.dwHighDateTime;
        kernel.LowPart = kernelTime.dwLowDateTime;
        kernel.HighPart = kernelTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;
        user.HighPart = userTime.dwHighDateTime;

        uint64_t total = kernel.QuadPart + user.QuadPart;  // busy + idle (idle is inside kernel)
        uint64_t busy = total - idle.QuadPart;

        return SystemCpuTimes{
            .busyTicks = busy,
            .totalTicks = total
        };
    }
#endif
    return std::nullopt;
}

} // namespace ucf::service
