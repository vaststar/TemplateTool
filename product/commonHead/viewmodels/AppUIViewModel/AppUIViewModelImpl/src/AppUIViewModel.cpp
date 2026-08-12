#include "AppUIViewModel.h"
#include "LoggerDefine.h"

#include <ucf/services/ClientInfoService/IClientInfoService.h>

#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/AppUIViewModel/AppUIViewModelCreator.h>


namespace commonHead::viewModels{

namespace impl{
std::shared_ptr<IAppUIViewModel> createAppUIViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<AppUIViewModel>(commonHeadFramework);
}
} // namespace impl

AppUIViewModel::AppUIViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IAppUIViewModel(commonHeadFramework)
{
    APP_UI_VIEW_MODEL_LOG_DEBUG("create AppUIViewModel");
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
    APP_UI_VIEW_MODEL_LOG_DEBUG("AppUIViewModel::initApplication start");
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto clientInfoService = serviceLocator->getClientInfoService().lock())
            {
                // Register first so we do not miss the Ready event between the
                // isReady probe and the initializeAppClient call.
                clientInfoService->registerCallback(shared_from_this());
                if (clientInfoService->isClientInfoReady())
                {
                    APP_UI_VIEW_MODEL_LOG_DEBUG("AppUIViewModel::initApplication: already ready, dispatching synchronously");
                    onClientInfoReady();
                }
                else
                {
                    clientInfoService->initializeAppClient();
                }
            }
        }
    }
    APP_UI_VIEW_MODEL_LOG_DEBUG("AppUIViewModel::initApplication finish");
}

void AppUIViewModel::onClientInfoReady()
{
    APP_UI_VIEW_MODEL_LOG_DEBUG("AppUIViewModel::onClientInfoReady, fire onAppConfigInitialized");
    fireNotification(&IAppUIViewModelCallback::onAppConfigInitialized);
}

void AppUIViewModel::onClientThemeChanged(ucf::service::model::ThemeType /*themeType*/)
{
    // Theme change handled by ClientInfoViewModel
}

}
