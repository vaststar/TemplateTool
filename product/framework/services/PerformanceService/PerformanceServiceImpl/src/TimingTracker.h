#pragma once

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/PerformanceService/PerformanceInfo.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace ucf::service {

/// Thread-safe timing tracker for performance measurement
class SERVICE_EXPORT TimingTracker
{
public:
    TimingTracker() = default;
    ~TimingTracker() = default;
    
    /// Begin timing an operation
    /// @return Token to be used with endTiming
    TimingToken beginTiming(const std::string& operationName);
    
    /// End timing and record the duration
    void endTiming(const TimingToken& token);
    
    /// Get statistics for a specific operation
    [[nodiscard]] TimingStats getStats(const std::string& operationName) const;
    
    /// Get all statistics
    [[nodiscard]] std::vector<TimingStats> getAllStats() const;
    
    /// Reset all statistics
    void reset();

private:
    mutable std::mutex mMutex;
    std::unordered_map<std::string, TimingStats> mStats;
    std::atomic<uint64_t> mNextTokenId{1};
};

} // namespace ucf::service
