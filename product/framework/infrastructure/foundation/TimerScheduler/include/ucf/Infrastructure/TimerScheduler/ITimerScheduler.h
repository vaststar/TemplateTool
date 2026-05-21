#pragma once

#include <chrono>
#include <functional>

#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>
#include <ucf/Infrastructure/TimerScheduler/TimerHandle.h>

namespace ucf::infrastructure::scheduling {

using TimerCallback = std::function<void()>;

// Schedules callbacks for delayed or periodic execution.
//
// Callbacks run sequentially on a single internal worker thread, so they
// must be short and non-blocking; long work delays every other timer.
// Exceptions escaping a callback are caught and logged.
//
// Obtain instances via TimerSchedulerFactory::create().
class Infrastructure_EXPORT ITimerScheduler
{
public:
    ITimerScheduler() = default;
    ITimerScheduler(const ITimerScheduler&) = delete;
    ITimerScheduler(ITimerScheduler&&) = delete;
    ITimerScheduler& operator=(const ITimerScheduler&) = delete;
    ITimerScheduler& operator=(ITimerScheduler&&) = delete;
    virtual ~ITimerScheduler() = default;

    // Run `callback` once after `delay`.
    virtual TimerHandle scheduleOnce(
        std::chrono::milliseconds delay,
        TimerCallback callback) = 0;

    // Run `callback` repeatedly on a fixed wall-clock cadence:
    // next fire = previous scheduled time + period. Missed ticks
    // (caused by slow callbacks) are skipped, never bursted.
    virtual TimerHandle scheduleAtFixedRate(
        std::chrono::milliseconds initialDelay,
        std::chrono::milliseconds period,
        TimerCallback callback) = 0;

    // Run `callback` repeatedly with a fixed gap between calls:
    // next fire = previous completion time + period.
    virtual TimerHandle scheduleWithFixedDelay(
        std::chrono::milliseconds initialDelay,
        std::chrono::milliseconds period,
        TimerCallback callback) = 0;

    // Cancel a scheduled task. Safe to call from inside a callback
    // (including the callback's own handle). A callback already in
    // progress is allowed to finish; periodic tasks will not re-fire.
    // Returns true if the handle was active and is now cancelled.
    virtual bool cancel(TimerHandle handle) = 0;

    // Stop the worker thread and discard pending tasks. Idempotent.
    // Blocks until the worker has exited (unless called from within
    // a callback, in which case the worker exits asynchronously).
    virtual void shutdown() = 0;
};

} // namespace ucf::infrastructure::scheduling
