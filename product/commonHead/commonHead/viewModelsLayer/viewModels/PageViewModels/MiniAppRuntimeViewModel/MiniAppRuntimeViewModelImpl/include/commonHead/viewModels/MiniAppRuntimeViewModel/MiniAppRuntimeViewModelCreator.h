#pragma once

#include <memory>

#include <commonHead/viewModels/MiniAppRuntimeViewModel/IMiniAppRuntimeViewModel.h>
#include <commonHead/viewModels/MiniAppRuntimeViewModel/MiniAppRuntimeViewModelImplExport.h>

namespace commonHead::viewModels::impl {

MINI_APP_RUNTIME_VIEW_MODEL_IMPL_API std::shared_ptr<IMiniAppRuntimeViewModel>
createMiniAppRuntimeViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
