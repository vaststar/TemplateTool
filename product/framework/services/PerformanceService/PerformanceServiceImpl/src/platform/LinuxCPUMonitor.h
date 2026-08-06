#pragma once

#include "../ICPUMonitor.h"
#include <cstdint>

namespace ucf::service {

class LinuxCPUMonitor : public ICPUMonitor
{
public:
    LinuxCPUMonitor();
    ~LinuxCPUMonitor() override = default;

    [[nodiscard]] uint64_t getProcessCpuTimeMicros() const override;
    [[nodiscard]] SystemCpuTimes getSystemCpuTimes() const override;
    [[nodiscard]] unsigned int getCpuCoreCount() const override;

private:
    /// Get process CPU time in clock ticks
    [[nodiscard]] uint64_t getProcessCpuTicks() const;

private:
    long mClockTicksPerSecond{100};
};

} // namespace ucf::service
