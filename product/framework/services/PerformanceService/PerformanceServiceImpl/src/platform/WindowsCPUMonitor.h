#pragma once

#include "../ICPUMonitor.h"
#include <cstdint>

namespace ucf::service {

class WindowsCPUMonitor : public ICPUMonitor
{
public:
    WindowsCPUMonitor() = default;
    ~WindowsCPUMonitor() override = default;

    [[nodiscard]] uint64_t getProcessCpuTimeMicros() const override;
    [[nodiscard]] SystemCpuTimes getSystemCpuTimes() const override;
    [[nodiscard]] unsigned int getCpuCoreCount() const override;
};

} // namespace ucf::service
