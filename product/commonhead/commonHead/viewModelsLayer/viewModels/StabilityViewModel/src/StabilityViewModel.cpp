#include "StabilityViewModel.h"

#include <ucf/Services/StabilityService/IStabilityService.h>
#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

namespace commonHead::viewModels{

std::shared_ptr<IStabilityViewModel> IStabilityViewModel::createInstance(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<StabilityViewModel>(commonHeadFramework);
}

StabilityViewModel::~StabilityViewModel()
{
    COMMONHEAD_LOG_DEBUG("");
}

StabilityViewModel::StabilityViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IStabilityViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create StabilityViewModel");
}

std::string StabilityViewModel::getViewModelName() const
{
    return "StabilityViewModel";
}

void StabilityViewModel::init()
{
    // No initialization needed
}

void StabilityViewModel::reportHeartbeat()
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto stabilityService = serviceLocator->getStabilityService().lock())
            {
                stabilityService->reportHeartbeat();
            }
        }
    }
}

int StabilityViewModel::getHeartbeatIntervalMs() const
{
    return DEFAULT_HEARTBEAT_INTERVAL_MS;
}

} // namespace commonHead::viewModels
