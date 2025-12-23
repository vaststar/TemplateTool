#include "MainWindowViewModel.h"

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <ucf/Services/InvocationService/IInvocationService.h>

namespace commonHead::viewModels{
std::shared_ptr<IMainWindowViewModel> IMainWindowViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MainWindowViewModel>(commonHeadFramework);
}

MainWindowViewModel::MainWindowViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IMainWindowViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create MainWindowViewModel");
}

std::string MainWindowViewModel::getViewModelName() const
{
    return "MainWindowViewModel";
}

void MainWindowViewModel::init()
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto service = serviceLocator->getInvocationService().lock())
            {
                service->registerCallback(shared_from_this());
            }
        }
    }

}
void MainWindowViewModel::onCommandMessageReceived(const std::string& message)
{
    if (message == "ActivateWindow")
    {
        fireNotification(&IMainWindowViewModelCallback::onActivateMainWindow);
    }
}
}