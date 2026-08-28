#pragma once

#include <memory>

#include <commonhead/viewmodels/Base64ToolViewModel/Base64ToolViewModelImplExport.h>
#include <commonhead/viewmodels/Base64ToolViewModel/IBase64ToolViewModel.h>

namespace commonHead::viewModels::impl {

BASE64_TOOL_VIEW_MODEL_IMPL_API std::shared_ptr<IBase64ToolViewModel>
createBase64ToolViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
