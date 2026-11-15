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
    APP_UI_VIEW_MODEL_LOG_DEBUG("AppUIViewModel constructed, address: " << this);
}

AppUIViewModel::~AppUIViewModel()
{
    APP_UI_VIEW_MODEL_LOG_DEBUG("AppUIViewModel destroying, address: " << this);
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
        publishInitializationFailure("CommonHeadFramework is not available");
        return;
    }

    auto serviceLocator = commonHeadFramework->getServiceLocator();
    if (!serviceLocator)
    {
        publishInitializationFailure("ServiceLocator is not available");
        return;
    }

    auto clientInfoService = serviceLocator->getClientInfoService().lock();
    if (!clientInfoService)
    {
        publishInitializationFailure("ClientInfoService is not available");
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
    if (mInitializationResultDelivered.exchange(true))
    {
        return;
    }
    APP_UI_VIEW_MODEL_LOG_DEBUG("AppUIViewModel::onClientInfoReady, fire onAppConfigInitialized");
    fireNotification(&IAppUIViewModelCallback::onAppConfigInitialized);
}

void AppUIViewModel::onClientInfoLoadFailed(ucf::service::ClientInfoLoadError error)
{
    using ucf::service::ClientInfoLoadError;
    switch (error)
    {
    case ClientInfoLoadError::DbServiceUnavailable:
        publishInitializationFailure("Client settings database service is not available");
        break;
    case ClientInfoLoadError::DbQueryFailed:
        publishInitializationFailure("Client settings database query failed");
        break;
    case ClientInfoLoadError::DbInitializationFailed:
        publishInitializationFailure("Client settings database initialization failed");
        break;
    }
}

void AppUIViewModel::publishInitializationFailure(const std::string& reason)
{
    if (mInitializationResultDelivered.exchange(true))
    {
        return;
    }
    APP_UI_VIEW_MODEL_LOG_ERROR("Application configuration initialization failed: " << reason);
    fireNotification(&IAppUIViewModelCallback::onAppConfigInitializationFailed, reason);
}

void AppUIViewModel::onClientThemeChanged(ucf::service::model::ThemeType /*themeType*/)
{
    // Theme change handled by ClientInfoViewModel
}

}
