#include <ucf/Infrastructure/TimerScheduler/TimerSchedulerFactory.h>

#include "TimerSchedulerImpl.h"

namespace ucf::infrastructure::scheduling {

std::unique_ptr<ITimerScheduler> TimerSchedulerFactory::create()
{
    return std::make_unique<TimerSchedulerImpl>();
}

} // namespace ucf::infrastructure::scheduling
