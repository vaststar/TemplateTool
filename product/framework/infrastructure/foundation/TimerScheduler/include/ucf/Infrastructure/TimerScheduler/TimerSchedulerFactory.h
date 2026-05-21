#pragma once

#include <memory>

#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>

namespace ucf::infrastructure::scheduling {

class ITimerScheduler;

class Infrastructure_EXPORT TimerSchedulerFactory final
{
public:
    TimerSchedulerFactory() = delete;

    // Create a scheduler with its own dedicated worker thread.
    // The thread starts immediately and runs until shutdown() or
    // until the returned object is destroyed.
    static std::unique_ptr<ITimerScheduler> create();
};

} // namespace ucf::infrastructure::scheduling
