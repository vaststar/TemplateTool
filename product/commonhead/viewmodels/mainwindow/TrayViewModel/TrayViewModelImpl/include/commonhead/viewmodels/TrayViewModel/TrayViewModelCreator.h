#pragma once

#include <memory>

#include <commonhead/viewmodels/TrayViewModel/ITrayViewModel.h>
#include <commonhead/viewmodels/TrayViewModel/TrayViewModelImplExport.h>

namespace commonHead::viewModels::impl {

TRAY_VIEW_MODEL_IMPL_API std::shared_ptr<ITrayViewModel> createTrayViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
