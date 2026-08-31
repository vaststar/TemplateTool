#include "LinuxCPUMonitor.h"

#include <fstream>
#include <sstream>
#include <unistd.h>

namespace ucf::service {

LinuxCPUMonitor::LinuxCPUMonitor()
    : mClockTicksPerSecond(sysconf(_SC_CLK_TCK))
{
}

std::optional<ProcessCpuTime> LinuxCPUMonitor::getProcessCpuTime() const
{
    if (mClockTicksPerSecond <= 0)
    {
        return std::nullopt;
    }

    const auto ticks = getProcessCpuTicks();
    if (!ticks)
    {
        return std::nullopt;
    }

    // ticks -> microseconds
    return ProcessCpuTime{
        (*ticks * 1000000ULL) / static_cast<uint64_t>(mClockTicksPerSecond)
    };
}

std::optional<SystemCpuTimes> LinuxCPUMonitor::getSystemCpuTimes() const
{
    if (mClockTicksPerSecond <= 0)
    {
        return std::nullopt;
    }

    // First line of /proc/stat: "cpu user nice system idle iowait irq softirq steal guest guest_nice"
    std::ifstream stat("/proc/stat");
    if (!stat.is_open())
    {
        return std::nullopt;
    }

    std::string cpuLabel;
    stat >> cpuLabel;
    if (cpuLabel != "cpu")
    {
        return std::nullopt;
    }

    uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0,
             irq = 0, softirq = 0, steal = 0, guest = 0, guestNice = 0;
    stat >> user >> nice >> system >> idle >> iowait
         >> irq >> softirq >> steal >> guest >> guestNice;
    if (!stat)
    {
        return std::nullopt;
    }

    uint64_t idleTicks = idle + iowait;
    uint64_t totalTicks = user + nice + system + idle + iowait + irq + softirq + steal;
    uint64_t busyTicks = (totalTicks > idleTicks) ? (totalTicks - idleTicks) : 0;

    return SystemCpuTimes{
        .busyTicks = busyTicks,
        .totalTicks = totalTicks
    };
}

std::optional<uint64_t> LinuxCPUMonitor::getProcessCpuTicks() const
{
    // Read from /proc/self/stat
    std::ifstream stat("/proc/self/stat");
    if (!stat.is_open())
    {
        return std::nullopt;
    }

    std::string line;
    std::getline(stat, line);

    // Skip past the comm field (which may contain spaces in parentheses)
    size_t start = line.rfind(')');
    if (start == std::string::npos)
    {
        return std::nullopt;
    }

    std::istringstream iss(line.substr(start + 2));

    // Fields after (comm): state, ppid, pgrp, session, tty_nr, tpgid, flags,
    // minflt, cminflt, majflt, cmajflt, utime(14), stime(15), cutime, cstime
    std::string state;
    long ppid, pgrp, session, tty_nr, tpgid;
    unsigned long flags, minflt, cminflt, majflt, cmajflt;
    unsigned long utime, stime;

    iss >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags
        >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime;
    if (!iss)
    {
        return std::nullopt;
    }

    return utime + stime;
}

} // namespace ucf::service
