#pragma once

#include <memory>

#include <commonhead/viewmodels/MiniAppRuntimeViewModel/IMiniAppRuntimeViewModel.h>
#include <commonhead/viewmodels/MiniAppRuntimeViewModel/MiniAppRuntimeViewModelImplExport.h>

namespace commonHead::viewModels::impl {

MINI_APP_RUNTIME_VIEW_MODEL_IMPL_API std::shared_ptr<IMiniAppRuntimeViewModel>
createMiniAppRuntimeViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
