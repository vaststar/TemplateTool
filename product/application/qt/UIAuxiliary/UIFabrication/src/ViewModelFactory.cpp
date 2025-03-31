#include "ViewModelFactory.h"
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

namespace UIFabrication{

std::unique_ptr<IViewModelFactory> IViewModelFactory::createInstance(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework)
{
    return std::make_unique<ViewModelFactory>(commonHeadFramework);
}

ViewModelFactory::ViewModelFactory(commonHead::ICommonHeadFrameworkWPtr commonHeadFramework)
    : mCommonHeadFramework(commonHeadFramework)
{

}

ViewModelFactory::~ViewModelFactory()
{
    
}

std::shared_ptr<commonHead::viewModels::IAppUIViewModel> ViewModelFactory::createAppUIViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::IAppUIViewModel>();
}

std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> ViewModelFactory::createClientInfoViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::IClientInfoViewModel>();
}

std::shared_ptr<commonHead::viewModels::IContactListViewModel> ViewModelFactory::createContactListViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::IContactListViewModel>();
}

std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> ViewModelFactory::createMainWindowViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::IMainWindowViewModel>();
}

std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> ViewModelFactory::createMediaCameraViewModelInstance() const
{
    return createViewModelInstance<commonHead::viewModels::IMediaCameraViewModel>();
}
} // namespace UIFabrication