#pragma once

#include <memory>

#include <commonhead/viewmodels/AppUIViewModel/IAppUIViewModel.h>
#include <commonhead/viewmodels/AppUIViewModel/AppUIViewModelImplExport.h>

namespace commonHead::viewModels::impl{

APP_UI_VIEW_MODEL_IMPL_API std::shared_ptr<IAppUIViewModel> createAppUIViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
