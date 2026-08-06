#pragma once

#include "../ICPUMonitor.h"
#include <cstdint>

namespace ucf::service {

class MacOSCPUMonitor : public ICPUMonitor
{
public:
    MacOSCPUMonitor() = default;
    ~MacOSCPUMonitor() override = default;

    [[nodiscard]] uint64_t getProcessCpuTimeMicros() const override;
    [[nodiscard]] SystemCpuTimes getSystemCpuTimes() const override;
    [[nodiscard]] unsigned int getCpuCoreCount() const override;
};

} // namespace ucf::service
