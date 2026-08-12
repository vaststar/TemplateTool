#pragma once

#include <memory>

#include <commonhead/viewmodels/ToolsViewModel/IToolsViewModel.h>
#include <commonhead/viewmodels/ToolsViewModel/ToolsViewModelImplExport.h>

namespace commonHead::viewModels::impl {

TOOLS_VIEW_MODEL_IMPL_API std::shared_ptr<IToolsViewModel>
createToolsViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
