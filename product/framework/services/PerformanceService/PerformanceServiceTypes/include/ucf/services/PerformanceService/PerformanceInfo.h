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

/// Timing statistics for a specific operation
struct PERFORMANCE_SERVICE_TYPES_API TimingStats {
    std::string operationName;
    uint64_t callCount{0};
    std::chrono::milliseconds totalTime{0};
    std::chrono::milliseconds minTime{std::chrono::milliseconds::max()};
    std::chrono::milliseconds maxTime{0};

    [[nodiscard]] std::chrono::milliseconds avgTime() const {
        return callCount > 0 ? std::chrono::milliseconds(totalTime.count() / callCount)
                             : std::chrono::milliseconds{0};
    }
};

/// Token returned by beginTiming, used to end timing
struct PERFORMANCE_SERVICE_TYPES_API TimingToken {
    uint64_t id{0};
    std::string operationName;
    std::chrono::steady_clock::time_point startTime;

    [[nodiscard]] bool isValid() const { return id != 0; }
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
