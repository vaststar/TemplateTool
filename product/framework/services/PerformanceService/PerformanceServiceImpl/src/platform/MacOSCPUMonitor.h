#pragma once

#include "../ICPUMonitor.h"
namespace ucf::service {

class MacOSCPUMonitor : public ICPUMonitor
{
public:
    MacOSCPUMonitor() = default;
    ~MacOSCPUMonitor() override = default;

    [[nodiscard]] std::optional<ProcessCpuTime> getProcessCpuTime() const override;
    [[nodiscard]] std::optional<SystemCpuTimes> getSystemCpuTimes() const override;
};

} // namespace ucf::service
