#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace ucf::service {

/// Memory usage information
struct MemoryInfo {
    uint64_t physicalBytes{0};       ///< Physical memory used by process
    uint64_t virtualBytes{0};        ///< Virtual memory used by process
    uint64_t peakPhysicalBytes{0};   ///< Peak physical memory usage
    uint64_t availableSystemBytes{0}; ///< Available system memory
};

/// Timing statistics for a specific operation
struct TimingStats {
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
struct TimingToken {
    uint64_t id{0};
    std::string operationName;
    std::chrono::steady_clock::time_point startTime;
    
    [[nodiscard]] bool isValid() const { return id != 0; }
};

/// Complete performance snapshot
struct PerformanceSnapshot {
    std::chrono::system_clock::time_point timestamp;
    MemoryInfo memory;
    double cpuUsagePercent{0.0};
    std::vector<TimingStats> timingStats;
};

} // namespace ucf::service
