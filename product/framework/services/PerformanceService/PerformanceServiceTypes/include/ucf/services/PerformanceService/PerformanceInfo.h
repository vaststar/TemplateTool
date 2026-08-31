#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <ucf/services/PerformanceService/PerformanceServiceTypesExport.h>

namespace ucf::service {

/// Memory usage information. A field is nullopt when the current platform does
/// not expose that metric or the sample could not be obtained.
struct PERFORMANCE_SERVICE_TYPES_API MemoryInfo {
    std::optional<uint64_t> processResidentBytes;            ///< Current resident set size
    std::optional<uint64_t> processPeakResidentBytes;        ///< Peak resident set size
    std::optional<uint64_t> processVirtualAddressSpaceBytes; ///< Process virtual address-space size
    std::optional<uint64_t> processPrivateCommittedBytes;    ///< Private committed process memory
    std::optional<uint64_t> systemAvailablePhysicalBytes;    ///< Estimated physical memory currently available
};

using TimingDuration = std::chrono::microseconds;

/// Timing statistics for a specific operation.
struct PERFORMANCE_SERVICE_TYPES_API TimingStats {
    std::string operationName;
    uint64_t callCount{0};
    TimingDuration totalDuration{0};
    std::optional<TimingDuration> minimumDuration;
    std::optional<TimingDuration> maximumDuration;

    [[nodiscard]] std::optional<TimingDuration> averageDuration() const
    {
        if (callCount == 0)
        {
            return std::nullopt;
        }

        return TimingDuration{
            totalDuration.count() / static_cast<TimingDuration::rep>(callCount)
        };
    }
};

/// Opaque identifier returned by beginTiming and consumed by endTiming.
struct PERFORMANCE_SERVICE_TYPES_API TimingToken {
    uint64_t id{0};

    [[nodiscard]] bool isValid() const noexcept { return id != 0; }
};

/// Complete performance snapshot
struct PERFORMANCE_SERVICE_TYPES_API PerformanceSnapshot {
    std::chrono::system_clock::time_point timestamp;
    MemoryInfo memory;
    std::optional<double> processCpuUsagePercent;
    std::optional<double> systemCpuUsagePercent;
    std::vector<TimingStats> timingStats;
};

} // namespace ucf::service
