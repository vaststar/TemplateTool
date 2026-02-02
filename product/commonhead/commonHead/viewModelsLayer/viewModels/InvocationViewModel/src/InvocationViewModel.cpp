#include "InvocationViewModel.h"

#include <ucf/Services/InvocationService/IInvocationService.h>


#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

namespace commonHead::viewModels{
std::shared_ptr<IInvocationViewModel> IInvocationViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<InvocationViewModel>(commonHeadFramework);
}

InvocationViewModel::~InvocationViewModel()
{
    COMMONHEAD_LOG_DEBUG("");
    
}

InvocationViewModel::InvocationViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IInvocationViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create InvocationViewModel");
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
                COMMONHEAD_LOG_DEBUG("will processed command message: " << message);
                invocationService->processCommandMessage(message);
                COMMONHEAD_LOG_DEBUG("finish processed command message: " << message);
            }
        }
    }
}
}
