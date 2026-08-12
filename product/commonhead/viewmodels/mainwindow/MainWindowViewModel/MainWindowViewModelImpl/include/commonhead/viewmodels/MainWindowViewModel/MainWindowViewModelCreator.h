#pragma once

#include <memory>

#include <commonhead/viewmodels/MainWindowViewModel/IMainWindowViewModel.h>
#include <commonhead/viewmodels/MainWindowViewModel/MainWindowViewModelImplExport.h>

namespace commonHead::viewModels::impl{

MAIN_WINDOW_VIEW_MODEL_IMPL_API std::shared_ptr<IMainWindowViewModel> createMainWindowViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
