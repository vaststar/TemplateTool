#include "MacOSCPUMonitor.h"

#include <mach/mach.h>
#include <sys/resource.h>

#include <limits>

namespace ucf::service {
namespace {

std::optional<ProcessCpuTime> toProcessCpuTime(const timeval& value)
{
    if (value.tv_sec < 0 || value.tv_usec < 0 || value.tv_usec >= 1000000)
    {
        return std::nullopt;
    }

    const auto seconds = static_cast<uint64_t>(value.tv_sec);
    const auto microseconds = static_cast<uint64_t>(value.tv_usec);
    if (seconds > (std::numeric_limits<uint64_t>::max() - microseconds) / 1000000ULL)
    {
        return std::nullopt;
    }

    return ProcessCpuTime{seconds * 1000000ULL + microseconds};
}

} // namespace

std::optional<ProcessCpuTime> MacOSCPUMonitor::getProcessCpuTime() const
{
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) != 0)
    {
        return std::nullopt;
    }

    const auto userTime = toProcessCpuTime(usage.ru_utime);
    const auto systemTime = toProcessCpuTime(usage.ru_stime);
    if (!userTime || !systemTime ||
        userTime->count() > std::numeric_limits<uint64_t>::max() - systemTime->count())
    {
        return std::nullopt;
    }

    return ProcessCpuTime{userTime->count() + systemTime->count()};
}

std::optional<SystemCpuTimes> MacOSCPUMonitor::getSystemCpuTimes() const
{
    host_cpu_load_info_data_t cpuLoad{};
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(),
                        HOST_CPU_LOAD_INFO,
                        reinterpret_cast<host_info_t>(&cpuLoad),
                        &count) != KERN_SUCCESS)
    {
        return std::nullopt;
    }

    uint64_t userTicks   = cpuLoad.cpu_ticks[CPU_STATE_USER];
    uint64_t niceTicks   = cpuLoad.cpu_ticks[CPU_STATE_NICE];
    uint64_t systemTicks = cpuLoad.cpu_ticks[CPU_STATE_SYSTEM];
    uint64_t idleTicks   = cpuLoad.cpu_ticks[CPU_STATE_IDLE];

    return SystemCpuTimes{
        .busyTicks = userTicks + niceTicks + systemTicks,
        .totalTicks = userTicks + niceTicks + systemTicks + idleTicks
    };
}

} // namespace ucf::service
