#pragma once

#include <memory>

#include <commonhead/viewmodels/JsonToolViewModel/IJsonToolViewModel.h>
#include <commonhead/viewmodels/JsonToolViewModel/JsonToolViewModelImplExport.h>

namespace commonHead::viewModels::impl {

JSON_TOOL_VIEW_MODEL_IMPL_API std::shared_ptr<IJsonToolViewModel> createJsonToolViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
