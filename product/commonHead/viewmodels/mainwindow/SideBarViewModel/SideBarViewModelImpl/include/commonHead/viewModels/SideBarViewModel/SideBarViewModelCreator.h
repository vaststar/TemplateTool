#pragma once

#include <memory>

#include <commonHead/viewModels/SideBarViewModel/ISideBarViewModel.h>
#include <commonHead/viewModels/SideBarViewModel/SideBarViewModelImplExport.h>

namespace commonHead::viewModels::impl {

SIDE_BAR_VIEW_MODEL_IMPL_API std::shared_ptr<ISideBarViewModel> createSideBarViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
