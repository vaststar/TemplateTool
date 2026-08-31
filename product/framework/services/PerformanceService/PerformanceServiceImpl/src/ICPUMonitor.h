#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>

namespace ucf::service {

using ProcessCpuTime = std::chrono::duration<uint64_t, std::micro>;

/// Cumulative system-wide CPU counters, used to compute overall CPU usage via deltas.
struct SystemCpuTimes {
    uint64_t busyTicks{0};   ///< Cumulative non-idle counter
    uint64_t totalTicks{0};  ///< Cumulative busy + idle counter
};

/// Interface for platform-specific CPU time sampling.
/// This is a stateless value-getter. Delta calculation, sampling cadence and
/// usage percentages are owned by PerformanceManager.
class ICPUMonitor
{
public:
    virtual ~ICPUMonitor() = default;

    /// Get monotonic cumulative process CPU time (user + system).
    /// Returns nullopt when the platform sample cannot be obtained.
    [[nodiscard]] virtual std::optional<ProcessCpuTime> getProcessCpuTime() const = 0;

    /// Get cumulative system-wide CPU counters. Both counters must use the same
    /// platform unit; only their deltas and ratio are consumed by the manager.
    /// Returns nullopt when the platform sample cannot be obtained.
    [[nodiscard]] virtual std::optional<SystemCpuTimes> getSystemCpuTimes() const = 0;

    /// Factory method
    [[nodiscard]] static std::unique_ptr<ICPUMonitor> create();
};

} // namespace ucf::service
