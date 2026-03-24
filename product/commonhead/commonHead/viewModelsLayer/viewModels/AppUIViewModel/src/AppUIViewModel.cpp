#include "AppUIViewModel.h"

#include <ucf/Services/ClientInfoService/IClientInfoService.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>


namespace commonHead::viewModels{
std::shared_ptr<IAppUIViewModel> IAppUIViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<AppUIViewModel>(commonHeadFramework);
}

AppUIViewModel::AppUIViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IAppUIViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create AppUIViewModel");
}

std::string AppUIViewModel::getViewModelName() const
{
    return "AppUIViewModel";
}

void AppUIViewModel::init()
{

}

void AppUIViewModel::initApplication()
{
    COMMONHEAD_LOG_DEBUG("AppUIViewModel::initApplication start");
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto clientInfoService = serviceLocator->getClientInfoService().lock())
            {
                clientInfoService->registerCallback(shared_from_this());
                clientInfoService->initializeAppClient();
            }
        }
    }
    COMMONHEAD_LOG_DEBUG("AppUIViewModel::initApplication finish");
}

void AppUIViewModel::onClientInfoReady()
{
    COMMONHEAD_LOG_DEBUG("AppUIViewModel::onClientInfoReady, fire onAppConfigInitialized");
    fireNotification(&IAppUIViewModelCallback::onAppConfigInitialized);
}

void AppUIViewModel::onClientThemeChanged(ucf::service::model::ThemeType /*themeType*/)
{
    // Theme change handled by ClientInfoViewModel
}

}
