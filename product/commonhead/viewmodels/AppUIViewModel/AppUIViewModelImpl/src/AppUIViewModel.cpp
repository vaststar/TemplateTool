#include "AppUIViewModel.h"
#include "LoggerDefine.h"

#include <ucf/services/ClientInfoService/IClientInfoService.h>

#include <commonhead/ServiceLocator/IServiceLocator.h>
#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/viewmodels/AppUIViewModel/AppUIViewModelCreator.h>


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
    auto commonHeadFramework = getCommonHeadFramework().lock();
    if (!commonHeadFramework)
    {
        APP_UI_VIEW_MODEL_LOG_ERROR(
            "AppUIViewModel::init: CommonHeadFramework is not available");
        return;
    }

    auto serviceLocator = commonHeadFramework->getServiceLocator();
    if (!serviceLocator)
    {
        APP_UI_VIEW_MODEL_LOG_ERROR(
            "AppUIViewModel::init: ServiceLocator is not available");
        return;
    }

    auto clientInfoService = serviceLocator->getClientInfoService().lock();
    if (!clientInfoService)
    {
        APP_UI_VIEW_MODEL_LOG_ERROR(
            "AppUIViewModel::init: ClientInfoService is not available");
        return;
    }

    // Register first so we do not miss the Ready event between the
    // isReady probe and the initializeAppClient call.
    clientInfoService->registerCallback(shared_from_this());
    if (clientInfoService->isClientInfoReady())
    {
        APP_UI_VIEW_MODEL_LOG_DEBUG(
            "AppUIViewModel::init: ClientInfoService is already ready, dispatching synchronously");
        onClientInfoReady();
        return;
    }

    APP_UI_VIEW_MODEL_LOG_DEBUG(
        "AppUIViewModel::init: requesting ClientInfoService initialization");
    clientInfoService->initializeAppClient();
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
