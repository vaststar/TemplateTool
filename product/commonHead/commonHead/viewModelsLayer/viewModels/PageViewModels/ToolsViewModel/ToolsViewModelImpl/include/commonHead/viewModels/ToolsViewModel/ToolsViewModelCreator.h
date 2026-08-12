#pragma once

#include <memory>

#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/ToolsViewModel/ToolsViewModelImplExport.h>

namespace commonHead::viewModels::impl {

TOOLS_VIEW_MODEL_IMPL_API std::shared_ptr<IToolsViewModel>
createToolsViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
