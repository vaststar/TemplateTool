#include "MacOSCPUMonitor.h"

#include <mach/mach.h>
#include <mach/task_info.h>
#include <sys/sysctl.h>
#include <thread>
#include <unistd.h>

namespace ucf::service {

unsigned int MacOSCPUMonitor::getCpuCoreCount() const
{
    return std::thread::hardware_concurrency();
}

uint64_t MacOSCPUMonitor::getProcessCpuTimeMicros() const
{
    task_thread_times_info_data_t timeInfo;
    mach_msg_type_number_t count = TASK_THREAD_TIMES_INFO_COUNT;

    kern_return_t kr = task_info(mach_task_self(),
                                  TASK_THREAD_TIMES_INFO,
                                  reinterpret_cast<task_info_t>(&timeInfo),
                                  &count);

    if (kr != KERN_SUCCESS)
    {
        return 0;
    }

    // Convert to microseconds
    uint64_t userTime = static_cast<uint64_t>(timeInfo.user_time.seconds) * 1000000 +
                        static_cast<uint64_t>(timeInfo.user_time.microseconds);
    uint64_t systemTime = static_cast<uint64_t>(timeInfo.system_time.seconds) * 1000000 +
                          static_cast<uint64_t>(timeInfo.system_time.microseconds);

    return userTime + systemTime;
}

SystemCpuTimes MacOSCPUMonitor::getSystemCpuTimes() const
{
    SystemCpuTimes times;

    host_cpu_load_info_data_t cpuLoad;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    kern_return_t kr = host_statistics(mach_host_self(),
                                       HOST_CPU_LOAD_INFO,
                                       reinterpret_cast<host_info_t>(&cpuLoad),
                                       &count);
    if (kr != KERN_SUCCESS)
    {
        return times;
    }

    uint64_t userTicks   = cpuLoad.cpu_ticks[CPU_STATE_USER];
    uint64_t niceTicks   = cpuLoad.cpu_ticks[CPU_STATE_NICE];
    uint64_t systemTicks = cpuLoad.cpu_ticks[CPU_STATE_SYSTEM];
    uint64_t idleTicks   = cpuLoad.cpu_ticks[CPU_STATE_IDLE];

    uint64_t totalTicks = userTicks + niceTicks + systemTicks + idleTicks;
    uint64_t busyTicks  = userTicks + niceTicks + systemTicks;

    // CPU load ticks are in units of sysconf(_SC_CLK_TCK)
    long clkTck = sysconf(_SC_CLK_TCK);
    const uint64_t clk = (clkTck > 0) ? static_cast<uint64_t>(clkTck) : 100ULL;

    times.totalMicros = (totalTicks * 1000000ULL) / clk;
    times.busyMicros = (busyTicks * 1000000ULL) / clk;
    return times;
}

} // namespace ucf::service
