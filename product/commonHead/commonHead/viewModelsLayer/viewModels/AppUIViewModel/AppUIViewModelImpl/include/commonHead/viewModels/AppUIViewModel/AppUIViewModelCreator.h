#pragma once

#include <memory>

#include <commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h>
#include <commonHead/viewModels/AppUIViewModel/AppUIViewModelImplExport.h>

namespace commonHead::viewModels::impl{

APP_UI_VIEW_MODEL_IMPL_API std::shared_ptr<IAppUIViewModel> createAppUIViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
