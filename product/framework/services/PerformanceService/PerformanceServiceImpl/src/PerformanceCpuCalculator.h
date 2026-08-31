#pragma once

#include "ICPUMonitor.h"

#include <chrono>
#include <cstdint>
#include <optional>

namespace ucf::service {

enum class CpuSampleStatus
{
    Unavailable,
    BaselineEstablished,
    Valid,
    CounterReset,
    Invalid
};

struct ProcessCpuUsageSample
{
    std::optional<double> percent;
    CpuSampleStatus status{CpuSampleStatus::Unavailable};
    ProcessCpuTime previousTime{0};
    ProcessCpuTime currentTime{0};
};

struct SystemCpuUsageSample
{
    std::optional<double> percent;
    CpuSampleStatus status{CpuSampleStatus::Unavailable};
    SystemCpuTimes previousTimes;
    SystemCpuTimes currentTimes;
};

struct CpuUsageSample
{
    ProcessCpuUsageSample process;
    SystemCpuUsageSample system;
};

/// Converts cumulative platform CPU counters into percentage samples.
/// Process CPU follows the one-logical-core-equals-100% convention and may
/// therefore exceed 100% for a multi-threaded process. System CPU is normalized
/// to the whole-machine 0%-100% scale.
class PerformanceCpuCalculator
{
public:
    [[nodiscard]] CpuUsageSample update(
        std::optional<ProcessCpuTime> processTime,
        std::optional<SystemCpuTimes> systemTimes,
        std::chrono::steady_clock::time_point wallTime);

private:
    std::optional<ProcessCpuTime> mPreviousProcessTime;
    std::optional<std::chrono::steady_clock::time_point> mPreviousProcessWallTime;
    std::optional<SystemCpuTimes> mPreviousSystemTimes;
};

/// Accumulates only valid samples for a report window.
class CpuUsageWindow
{
public:
    void add(std::optional<double> sample);
    [[nodiscard]] std::optional<double> average() const;
    [[nodiscard]] uint64_t sampleCount() const;
    void reset();

private:
    double mSum{0.0};
    uint64_t mSampleCount{0};
};

} // namespace ucf::service
