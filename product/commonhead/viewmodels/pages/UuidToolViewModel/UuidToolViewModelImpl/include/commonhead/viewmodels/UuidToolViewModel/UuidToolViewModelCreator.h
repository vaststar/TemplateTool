#pragma once

#include <memory>

#include <commonhead/viewmodels/UuidToolViewModel/IUuidToolViewModel.h>
#include <commonhead/viewmodels/UuidToolViewModel/UuidToolViewModelImplExport.h>

namespace commonHead::viewModels::impl {

UUID_TOOL_VIEW_MODEL_IMPL_API std::shared_ptr<IUuidToolViewModel>
createUuidToolViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
