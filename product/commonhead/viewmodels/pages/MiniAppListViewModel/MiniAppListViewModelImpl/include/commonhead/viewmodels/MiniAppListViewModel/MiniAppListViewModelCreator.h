#pragma once

#include <memory>

#include <commonhead/viewmodels/MiniAppListViewModel/IMiniAppListViewModel.h>
#include <commonhead/viewmodels/MiniAppListViewModel/MiniAppListViewModelImplExport.h>

namespace commonHead::viewModels::impl {

MINI_APP_LIST_VIEW_MODEL_IMPL_API std::shared_ptr<IMiniAppListViewModel>
createMiniAppListViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
