#include "ViewModelFactory.h"
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/InvocationViewModel/InvocationViewModelCreator.h>
#include <commonHead/viewModels/AppUIViewModel/AppUIViewModelCreator.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoViewModelCreator.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/MainWindowViewModel/MainWindowViewModelCreator.h>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>
#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryViewModel.h>
#include <commonHead/viewModels/SideBarViewModel/SideBarViewModelCreator.h>
#include <commonHead/viewModels/SettingsViewModel/SettingsViewModelCreator.h>
#include <commonHead/viewModels/StabilityViewModel/StabilityViewModelCreator.h>
#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/JsonTreeViewModel/JsonTreeViewModelCreator.h>
#include <commonHead/viewModels/NetworkProxyViewModel/INetworkProxyViewModel.h>
#include <commonHead/viewModels/ScreenshotViewModel/IScreenshotViewModel.h>
#include <commonHead/viewModels/RecordingViewModel/IRecordingViewModel.h>
#include <commonHead/viewModels/UpgradeViewModel/UpgradeViewModelCreator.h>
#include <commonHead/viewModels/MiniAppListViewModel/IMiniAppListViewModel.h>
#include <commonHead/viewModels/MiniAppRuntimeViewModel/IMiniAppRuntimeViewModel.h>

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
    return createViewModelInstance<commonHead::viewModels::IContactListViewModel>();
}

std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> ViewModelFactory::createMainWindowViewModelInstance() const
{
    return impl::createMainWindowViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> ViewModelFactory::createMediaCameraViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::IMediaCameraViewModel>();
}

std::shared_ptr<commonHead::viewModels::ICameraDirectoryViewModel> ViewModelFactory::createCameraDirectoryViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::ICameraDirectoryViewModel>();
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
    return createViewModelInstance<commonHead::viewModels::IToolsViewModel>();
}

std::shared_ptr<commonHead::viewModels::IJsonTreeViewModel> ViewModelFactory::createJsonTreeViewModelInstance() const
{
    return impl::createJsonTreeViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::INetworkProxyViewModel> ViewModelFactory::createNetworkProxyViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::INetworkProxyViewModel>();
}

std::shared_ptr<commonHead::viewModels::IScreenshotViewModel> ViewModelFactory::createScreenshotViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::IScreenshotViewModel>();
}

std::shared_ptr<commonHead::viewModels::IRecordingViewModel> ViewModelFactory::createRecordingViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::IRecordingViewModel>();
}

std::shared_ptr<commonHead::viewModels::IUpgradeViewModel> ViewModelFactory::createUpgradeViewModelInstance() const
{
    return impl::createUpgradeViewModel(mCommonHeadFramework);
}

std::shared_ptr<commonHead::viewModels::IMiniAppListViewModel> ViewModelFactory::createMiniAppListViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::IMiniAppListViewModel>();
}

std::shared_ptr<commonHead::viewModels::IMiniAppRuntimeViewModel> ViewModelFactory::createMiniAppRuntimeViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::IMiniAppRuntimeViewModel>();
}
} // namespace commonHead::viewModels
