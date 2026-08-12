#include "InvocationViewModel.h"
#include "LoggerDefine.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <commonHead/viewModels/InvocationViewModel/InvocationViewModelCreator.h>
#include <ucf/services/InvocationService/IInvocationService.h>

namespace commonHead::viewModels{

namespace impl{
std::shared_ptr<IInvocationViewModel> createInvocationViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<InvocationViewModel>(commonHeadFramework);
}
} // namespace impl

InvocationViewModel::~InvocationViewModel()
{
    INVOCATION_VIEW_MODEL_LOG_DEBUG("");
}

InvocationViewModel::InvocationViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IInvocationViewModel(commonHeadFramework)
{
    INVOCATION_VIEW_MODEL_LOG_DEBUG("create InvocationViewModel");
}

std::string InvocationViewModel::getViewModelName() const
{
    return "InvocationViewModel";
}

void InvocationViewModel::init()
{
}

void InvocationViewModel::processStartupParameters()
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto invocationService = serviceLocator->getInvocationService().lock())
            {
                invocationService->processStartupParameters();
            }
        }
    }
}

std::vector<std::string> InvocationViewModel::getStartupParameters() const
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto invocationService = serviceLocator->getInvocationService().lock())
            {
                return invocationService->getStartupParameters();
            }
        }
    }
    return {};
}

void InvocationViewModel::processCommandMessage(const std::string& message)
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto invocationService = serviceLocator->getInvocationService().lock())
            {
                INVOCATION_VIEW_MODEL_LOG_DEBUG("will processed command message: " << message);
                invocationService->processCommandMessage(message);
                INVOCATION_VIEW_MODEL_LOG_DEBUG("finish processed command message: " << message);
            }
        }
    }
}

}
