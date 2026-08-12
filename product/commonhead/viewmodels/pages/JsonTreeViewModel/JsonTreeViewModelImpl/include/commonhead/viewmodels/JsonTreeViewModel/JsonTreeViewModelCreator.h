#pragma once

#include <memory>

#include <commonhead/viewmodels/JsonTreeViewModel/IJsonTreeViewModel.h>
#include <commonhead/viewmodels/JsonTreeViewModel/JsonTreeViewModelImplExport.h>

namespace commonHead::viewModels::impl {

JSON_TREE_VIEW_MODEL_IMPL_API std::shared_ptr<IJsonTreeViewModel> createJsonTreeViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
