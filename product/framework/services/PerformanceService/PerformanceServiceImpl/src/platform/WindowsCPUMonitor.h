#pragma once

#include "../ICPUMonitor.h"
namespace ucf::service {

class WindowsCPUMonitor : public ICPUMonitor
{
public:
    WindowsCPUMonitor() = default;
    ~WindowsCPUMonitor() override = default;

    [[nodiscard]] std::optional<ProcessCpuTime> getProcessCpuTime() const override;
    [[nodiscard]] std::optional<SystemCpuTimes> getSystemCpuTimes() const override;
};

} // namespace ucf::service
