#include "StabilityViewModel.h"
#include "LoggerDefine.h"

#include <ucf/services/StabilityService/IStabilityService.h>
#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ServiceLocator/IServiceLocator.h>
#include <commonhead/viewmodels/StabilityViewModel/StabilityViewModelCreator.h>

namespace commonHead::viewModels{

namespace impl{
std::shared_ptr<IStabilityViewModel> createStabilityViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<StabilityViewModel>(commonHeadFramework);
}
} // namespace impl

StabilityViewModel::~StabilityViewModel()
{
    STABILITY_VIEW_MODEL_LOG_DEBUG("");
}

StabilityViewModel::StabilityViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IStabilityViewModel(commonHeadFramework)
{
    STABILITY_VIEW_MODEL_LOG_DEBUG("create StabilityViewModel");
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
