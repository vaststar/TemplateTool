#pragma once

#include <chrono>
#include <stdexcept>
#include <string>

namespace ucf::service {

/// Maintains absolute sampling and reporting deadlines on a monotonic clock.
///
/// Deadlines advance from the original schedule rather than from the time the
/// previous work finished. If work is delayed, missed slots are skipped so the
/// monitor never emits a burst of catch-up samples or reports.
class PerformanceMonitoringSchedule final
{
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    PerformanceMonitoringSchedule(TimePoint startTime,
                                  Duration sampleInterval,
                                  Duration reportInterval)
        : mSampleInterval(requirePositive(sampleInterval, "sampleInterval"))
        , mReportInterval(requirePositive(reportInterval, "reportInterval"))
        , mNextSampleDeadline(startTime + mSampleInterval)
        , mNextReportDeadline(startTime + mReportInterval)
    {
    }

    [[nodiscard]] TimePoint nextSampleDeadline() const
    {
        return mNextSampleDeadline;
    }

    [[nodiscard]] TimePoint nextReportDeadline() const
    {
        return mNextReportDeadline;
    }

    void advanceSampleDeadline(TimePoint now)
    {
        advancePast(mNextSampleDeadline, mSampleInterval, now);
    }

    [[nodiscard]] bool consumeReportDeadline(TimePoint now)
    {
        if (now < mNextReportDeadline)
        {
            return false;
        }

        advancePast(mNextReportDeadline, mReportInterval, now);
        return true;
    }

private:
    static Duration requirePositive(Duration interval, const char* intervalName)
    {
        if (interval <= Duration::zero())
        {
            throw std::invalid_argument(
                std::string(intervalName) + " must be greater than zero");
        }
        return interval;
    }

    static void advancePast(TimePoint& deadline, Duration interval, TimePoint now)
    {
        if (deadline > now)
        {
            return;
        }

        const auto elapsedIntervals = (now - deadline) / interval;
        deadline += interval * (elapsedIntervals + 1);
    }

    const Duration mSampleInterval;
    const Duration mReportInterval;
    TimePoint mNextSampleDeadline;
    TimePoint mNextReportDeadline;
};

} // namespace ucf::service
