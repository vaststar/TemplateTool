#pragma once

#include <ucf/services/PerformanceService/PerformanceServiceImplExport.h>
#include <ucf/services/PerformanceService/PerformanceInfo.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ucf::service {

/// Thread-safe timing tracker for performance measurement
class PERFORMANCE_SERVICE_IMPL_API TimingTracker
{
public:
    TimingTracker() = default;
    ~TimingTracker() = default;

    /// Begin timing an operation
    /// @return Token to be used with endTiming
    [[nodiscard]] TimingToken beginTiming(const std::string& operationName);

    /// End timing and record the duration
    void endTiming(const TimingToken& token);

    /// Get statistics for a specific operation
    [[nodiscard]] std::optional<TimingStats> getStats(
        const std::string& operationName) const;

    /// Get all statistics
    [[nodiscard]] std::vector<TimingStats> getAllStats() const;

    /// Reset all statistics
    void reset();

private:
    struct ActiveTiming
    {
        std::string operationName;
        std::chrono::steady_clock::time_point startTime;
    };

private:
    inline static std::atomic<uint64_t> sNextTokenId{1};

    mutable std::mutex mMutex;
    std::unordered_map<uint64_t, ActiveTiming> mActiveTimings;
    std::unordered_map<std::string, TimingStats> mStats;
};

} // namespace ucf::service
