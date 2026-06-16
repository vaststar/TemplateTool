#pragma once

#include <cstdint>
#include <memory>

namespace ucf::service {

/// Cumulative system-wide CPU times, used to compute overall CPU usage via deltas.
struct SystemCpuTimes {
    uint64_t busyMicros{0};   ///< Time the whole system spent non-idle
    uint64_t totalMicros{0};  ///< Total time (busy + idle) across all cores
};

/// Interface for platform-specific CPU time sampling.
/// This is a stateless value-getter: it only exposes the cumulative process CPU
/// time. Delta calculation, sampling cadence and usage% are owned by
/// PerformanceManager's monitoring loop.
class ICPUMonitor
{
public:
    virtual ~ICPUMonitor() = default;

    /// Get cumulative process CPU time (user + system) in microseconds.
    /// Normalized across platforms so the manager can compute usage uniformly.
    [[nodiscard]] virtual uint64_t getProcessCpuTimeMicros() const = 0;

    /// Get cumulative system-wide CPU times (busy/total) in microseconds.
    /// The manager computes system CPU% from the delta of two samples.
    [[nodiscard]] virtual SystemCpuTimes getSystemCpuTimes() const = 0;

    /// Get the number of CPU cores
    [[nodiscard]] virtual unsigned int getCpuCoreCount() const = 0;

    /// Factory method
    [[nodiscard]] static std::unique_ptr<ICPUMonitor> create();
};

} // namespace ucf::service
