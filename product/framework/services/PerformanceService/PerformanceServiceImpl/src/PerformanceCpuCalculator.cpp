#include "PerformanceCpuCalculator.h"

#include <cmath>

namespace ucf::service {
namespace {

ProcessCpuUsageSample calculateProcessUsage(
    std::optional<ProcessCpuTime>& previousTime,
    std::optional<std::chrono::steady_clock::time_point>& previousWallTime,
    std::optional<ProcessCpuTime> currentTime,
    std::chrono::steady_clock::time_point currentWallTime)
{
    ProcessCpuUsageSample result;

    if (!currentTime)
    {
        return result;
    }

    result.currentTime = *currentTime;
    if (!previousTime || !previousWallTime)
    {
        previousTime = currentTime;
        previousWallTime = currentWallTime;
        result.status = CpuSampleStatus::BaselineEstablished;
        return result;
    }

    result.previousTime = *previousTime;
    if (*currentTime < *previousTime)
    {
        previousTime = currentTime;
        previousWallTime = currentWallTime;
        result.status = CpuSampleStatus::CounterReset;
        return result;
    }

    const auto wallTimeDelta = std::chrono::duration_cast<std::chrono::microseconds>(
        currentWallTime - *previousWallTime);
    if (wallTimeDelta.count() <= 0)
    {
        previousTime = currentTime;
        previousWallTime = currentWallTime;
        result.status = CpuSampleStatus::Invalid;
        return result;
    }

    const auto processTimeDelta = *currentTime - *previousTime;
    const double percent =
        static_cast<double>(processTimeDelta.count()) /
        static_cast<double>(wallTimeDelta.count()) * 100.0;

    previousTime = currentTime;
    previousWallTime = currentWallTime;
    if (!std::isfinite(percent) || percent < 0.0)
    {
        result.status = CpuSampleStatus::Invalid;
        return result;
    }

    result.percent = percent;
    result.status = CpuSampleStatus::Valid;
    return result;
}

SystemCpuUsageSample calculateSystemUsage(
    std::optional<SystemCpuTimes>& previousTimes,
    std::optional<SystemCpuTimes> currentTimes)
{
    SystemCpuUsageSample result;

    if (!currentTimes)
    {
        return result;
    }

    result.currentTimes = *currentTimes;
    if (!previousTimes)
    {
        previousTimes = currentTimes;
        result.status = CpuSampleStatus::BaselineEstablished;
        return result;
    }

    result.previousTimes = *previousTimes;
    if (currentTimes->busyTicks < previousTimes->busyTicks ||
        currentTimes->totalTicks < previousTimes->totalTicks)
    {
        previousTimes = currentTimes;
        result.status = CpuSampleStatus::CounterReset;
        return result;
    }

    const uint64_t busyDelta = currentTimes->busyTicks - previousTimes->busyTicks;
    const uint64_t totalDelta = currentTimes->totalTicks - previousTimes->totalTicks;
    previousTimes = currentTimes;

    if (totalDelta == 0 || busyDelta > totalDelta)
    {
        result.status = CpuSampleStatus::Invalid;
        return result;
    }

    const double percent =
        static_cast<double>(busyDelta) / static_cast<double>(totalDelta) * 100.0;
    if (!std::isfinite(percent) || percent < 0.0 || percent > 100.0)
    {
        result.status = CpuSampleStatus::Invalid;
        return result;
    }

    result.percent = percent;
    result.status = CpuSampleStatus::Valid;
    return result;
}

} // namespace

CpuUsageSample PerformanceCpuCalculator::update(
    std::optional<ProcessCpuTime> processTime,
    std::optional<SystemCpuTimes> systemTimes,
    std::chrono::steady_clock::time_point wallTime)
{
    CpuUsageSample result;
    result.process = calculateProcessUsage(
        mPreviousProcessTime,
        mPreviousProcessWallTime,
        processTime,
        wallTime);
    result.system = calculateSystemUsage(mPreviousSystemTimes, systemTimes);
    return result;
}

void CpuUsageWindow::add(std::optional<double> sample)
{
    if (!sample || !std::isfinite(*sample))
    {
        return;
    }

    mSum += *sample;
    ++mSampleCount;
}

std::optional<double> CpuUsageWindow::average() const
{
    if (mSampleCount == 0)
    {
        return std::nullopt;
    }
    return mSum / static_cast<double>(mSampleCount);
}

uint64_t CpuUsageWindow::sampleCount() const
{
    return mSampleCount;
}

void CpuUsageWindow::reset()
{
    mSum = 0.0;
    mSampleCount = 0;
}

} // namespace ucf::service
