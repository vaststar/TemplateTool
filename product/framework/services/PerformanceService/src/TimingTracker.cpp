#include "TimingTracker.h"
#include "PerformanceServiceLogger.h"

namespace ucf::service {

TimingToken TimingTracker::beginTiming(const std::string& operationName)
{
    TimingToken token;
    token.id = mNextTokenId.fetch_add(1);
    token.operationName = operationName;
    token.startTime = std::chrono::steady_clock::now();
    return token;
}

void TimingTracker::endTiming(const TimingToken& token)
{
    if (!token.isValid())
    {
        PERFORMANCE_LOG_WARN("endTiming called with invalid token");
        return;
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - token.startTime);
    
    std::lock_guard<std::mutex> lock(mMutex);
    
    auto& stats = mStats[token.operationName];
    if (stats.operationName.empty())
    {
        stats.operationName = token.operationName;
    }
    
    stats.callCount++;
    stats.totalTime += duration;
    
    if (duration < stats.minTime)
    {
        stats.minTime = duration;
    }
    if (duration > stats.maxTime)
    {
        stats.maxTime = duration;
    }
    
    PERFORMANCE_LOG_DEBUG("[TIMING] " << token.operationName << ": " << duration.count() << "ms");
}

TimingStats TimingTracker::getStats(const std::string& operationName) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    
    auto it = mStats.find(operationName);
    if (it != mStats.end())
    {
        return it->second;
    }
    
    TimingStats empty;
    empty.operationName = operationName;
    return empty;
}

std::vector<TimingStats> TimingTracker::getAllStats() const
{
    std::lock_guard<std::mutex> lock(mMutex);
    
    std::vector<TimingStats> result;
    result.reserve(mStats.size());
    
    for (const auto& [name, stats] : mStats)
    {
        result.push_back(stats);
    }
    
    return result;
}

void TimingTracker::reset()
{
    std::lock_guard<std::mutex> lock(mMutex);
    mStats.clear();
    PERFORMANCE_LOG_INFO("Timing statistics reset");
}

} // namespace ucf::service
