#pragma once

#include <memory>

#include <ucf/Infrastructure/TimerScheduler/TimerSchedulerExport.h>

namespace ucf::infrastructure::scheduling {

class ITimerScheduler;

class TIMER_SCHEDULER_API TimerSchedulerFactory final
{
public:
    TimerSchedulerFactory() = delete;

    // Create a scheduler with its own dedicated worker thread.
    // The thread starts immediately and runs until shutdown() or
    // until the returned object is destroyed.
    static std::unique_ptr<ITimerScheduler> create();
};

} // namespace ucf::infrastructure::scheduling
