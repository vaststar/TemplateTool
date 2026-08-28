#include "ViewModelFactory.h"
#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/viewmodels/InvocationViewModel/InvocationViewModelCreator.h>
#include <commonhead/viewmodels/AppUIViewModel/AppUIViewModelCreator.h>
#include <commonhead/viewmodels/ClientInfoViewModel/ClientInfoViewModelCreator.h>
#include <commonhead/viewmodels/ContactListViewModel/ContactListViewModelCreator.h>
#include <commonhead/viewmodels/MainWindowViewModel/MainWindowViewModelCreator.h>
#include <commonhead/viewmodels/MediaCameraViewModel/MediaCameraViewModelCreator.h>
#include <commonhead/viewmodels/CameraDirectoryViewModel/CameraDirectoryViewModelCreator.h>
#include <commonhead/viewmodels/SideBarViewModel/SideBarViewModelCreator.h>
#include <commonhead/viewmodels/SettingsViewModel/SettingsViewModelCreator.h>
#include <commonhead/viewmodels/StabilityViewModel/StabilityViewModelCreator.h>
#include <commonhead/viewmodels/ToolsViewModel/ToolsViewModelCreator.h>
#include <commonhead/viewmodels/Base64ToolViewModel/Base64ToolViewModelCreator.h>
#include <commonhead/viewmodels/JsonToolViewModel/JsonToolViewModelCreator.h>
#include <commonhead/viewmodels/UuidToolViewModel/UuidToolViewModelCreator.h>
#include <commonhead/viewmodels/NetworkProxyViewModel/NetworkProxyViewModelCreator.h>
#include <commonhead/viewmodels/ScreenshotViewModel/ScreenshotViewModelCreator.h>
#include <commonhead/viewmodels/RecordingViewModel/RecordingViewModelCreator.h>
#include <commonhead/viewmodels/UpgradeViewModel/UpgradeViewModelCreator.h>
#include <commonhead/viewmodels/MiniAppListViewModel/MiniAppListViewModelCreator.h>
#include <commonhead/viewmodels/MiniAppRuntimeViewModel/MiniAppRuntimeViewModelCreator.h>

namespace commonHead::viewModels{

std::shared_ptr<IViewModelFactory> IViewModelFactory::createInstance(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework)
{
    return std::make_shared<ViewModelFactory>(commonHeadFramework);
}

ViewModelFactory::ViewModelFactory(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework)
    : mCommonHeadFramework(commonHeadFramework)
{

}

ViewModelFactory::~ViewModelFactory()
{

}

std::shared_ptr<commonHead::viewModels::IInvocationViewModel> ViewModelFactory::createInvocationViewModelInstance() const
{
    return impl::createInvocationViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IAppUIViewModel> ViewModelFactory::createAppUIViewModelInstance() const
{
    return impl::createAppUIViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> ViewModelFactory::createClientInfoViewModelInstance() const
{
    return impl::createClientInfoViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IContactListViewModel> ViewModelFactory::createContactListViewModelInstance() const
{
    return impl::createContactListViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> ViewModelFactory::createMainWindowViewModelInstance() const
{
    return impl::createMainWindowViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> ViewModelFactory::createMediaCameraViewModelInstance() const
{
    return impl::createMediaCameraViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::ICameraDirectoryViewModel> ViewModelFactory::createCameraDirectoryViewModelInstance() const
{
    return impl::createCameraDirectoryViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::ISideBarViewModel> ViewModelFactory::createSideBarViewModelInstance() const
{
    return impl::createSideBarViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::ISettingsViewModel> ViewModelFactory::createSettingsViewModelInstance() const
{
    return impl::createSettingsViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IStabilityViewModel> ViewModelFactory::createStabilityViewModelInstance() const
{
    return impl::createStabilityViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IToolsViewModel> ViewModelFactory::createToolsViewModelInstance() const
{
    return impl::createToolsViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IBase64ToolViewModel> ViewModelFactory::createBase64ToolViewModelInstance() const
{
    return impl::createBase64ToolViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IJsonToolViewModel> ViewModelFactory::createJsonToolViewModelInstance() const
{
    return impl::createJsonToolViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IUuidToolViewModel> ViewModelFactory::createUuidToolViewModelInstance() const
{
    return impl::createUuidToolViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::INetworkProxyViewModel> ViewModelFactory::createNetworkProxyViewModelInstance() const
{
    return impl::createNetworkProxyViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IScreenshotViewModel> ViewModelFactory::createScreenshotViewModelInstance() const
{
    return impl::createScreenshotViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IRecordingViewModel> ViewModelFactory::createRecordingViewModelInstance() const
{
    return impl::createRecordingViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IUpgradeViewModel> ViewModelFactory::createUpgradeViewModelInstance() const
{
    return impl::createUpgradeViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IMiniAppListViewModel> ViewModelFactory::createMiniAppListViewModelInstance() const
{
    return impl::createMiniAppListViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IMiniAppRuntimeViewModel> ViewModelFactory::createMiniAppRuntimeViewModelInstance() const
{
    return impl::createMiniAppRuntimeViewModel(mCommonHeadFramework);
}
} // namespace commonHead::viewModels
