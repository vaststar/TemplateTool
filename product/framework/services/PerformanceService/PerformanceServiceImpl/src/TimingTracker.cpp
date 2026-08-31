#include "TimingTracker.h"
#include "PerformanceServiceLogger.h"

#include <algorithm>
#include <utility>

namespace ucf::service {

TimingToken TimingTracker::beginTiming(const std::string& operationName)
{
    TimingToken token;

    if (operationName.empty())
    {
        PERFORMANCE_LOG_WARN("Timing start rejected: operation name is empty");
    }
    else
    {
        while (token.id == 0)
        {
            token.id = sNextTokenId.fetch_add(1);
        }

        ActiveTiming activeTiming{
            .operationName = operationName,
            .startTime = std::chrono::steady_clock::now()
        };

        std::lock_guard<std::mutex> lock(mMutex);
        mActiveTimings.emplace(token.id, std::move(activeTiming));
    }

    return token;
}

void TimingTracker::endTiming(const TimingToken& token)
{
    bool recorded = false;
    std::string operationName;
    TimingDuration duration{0};

    if (!token.isValid())
    {
        PERFORMANCE_LOG_WARN("Timing completion rejected: token is invalid");
    }
    else
    {
        const auto endTime = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(mMutex);

            const auto activeIt = mActiveTimings.find(token.id);
            if (activeIt != mActiveTimings.end())
            {
                ActiveTiming activeTiming = std::move(activeIt->second);
                mActiveTimings.erase(activeIt);

                operationName = std::move(activeTiming.operationName);
                duration = std::chrono::duration_cast<TimingDuration>(
                    endTime - activeTiming.startTime);

                auto& stats = mStats[operationName];
                stats.operationName = operationName;
                ++stats.callCount;
                stats.totalDuration += duration;

                if (!stats.minimumDuration || duration < *stats.minimumDuration)
                {
                    stats.minimumDuration = duration;
                }
                if (!stats.maximumDuration || duration > *stats.maximumDuration)
                {
                    stats.maximumDuration = duration;
                }

                recorded = true;
            }
        }

        if (recorded)
        {
            PERFORMANCE_LOG_DEBUG(
                "Timing sample recorded, operation: " << operationName
                << ", duration: " << duration.count() << " us"
                << ", tokenId: " << token.id);
        }
        else
        {
            PERFORMANCE_LOG_WARN(
                "Timing completion rejected: token is unknown, already completed, or invalidated, tokenId: "
                << token.id);
        }
    }
}

std::optional<TimingStats> TimingTracker::getStats(const std::string& operationName) const
{
    std::lock_guard<std::mutex> lock(mMutex);

    const auto it = mStats.find(operationName);
    if (it != mStats.end())
    {
        return it->second;
    }

    return std::nullopt;
}

std::vector<TimingStats> TimingTracker::getAllStats() const
{
    std::vector<TimingStats> result;

    {
        std::lock_guard<std::mutex> lock(mMutex);

        result.reserve(mStats.size());
        for (const auto& entry : mStats)
        {
            result.push_back(entry.second);
        }
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const TimingStats& lhs, const TimingStats& rhs)
        {
            return lhs.operationName < rhs.operationName;
        });

    return result;
}

void TimingTracker::reset()
{
    size_t invalidatedActiveCount{0};

    {
        std::lock_guard<std::mutex> lock(mMutex);
        mStats.clear();
        invalidatedActiveCount = mActiveTimings.size();
        mActiveTimings.clear();
    }

    PERFORMANCE_LOG_INFO(
        "Timing statistics reset, invalidatedActiveTimings: "
        << invalidatedActiveCount);
}

} // namespace ucf::service
