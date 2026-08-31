#pragma once

#include "../ICPUMonitor.h"
namespace ucf::service {

class LinuxCPUMonitor : public ICPUMonitor
{
public:
    LinuxCPUMonitor();
    ~LinuxCPUMonitor() override = default;

    [[nodiscard]] std::optional<ProcessCpuTime> getProcessCpuTime() const override;
    [[nodiscard]] std::optional<SystemCpuTimes> getSystemCpuTimes() const override;

private:
    /// Get process CPU time in clock ticks
    [[nodiscard]] std::optional<uint64_t> getProcessCpuTicks() const;

private:
    long mClockTicksPerSecond{100};
};

} // namespace ucf::service
