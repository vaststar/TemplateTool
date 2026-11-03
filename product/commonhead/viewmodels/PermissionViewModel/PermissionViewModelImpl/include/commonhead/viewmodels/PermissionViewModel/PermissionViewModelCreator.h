#pragma once

#include <memory>

#include <commonhead/viewmodels/PermissionViewModel/IPermissionViewModel.h>
#include <commonhead/viewmodels/PermissionViewModel/PermissionViewModelImplExport.h>

namespace commonHead::viewModels::impl{

PERMISSION_VIEW_MODEL_IMPL_API std::shared_ptr<IPermissionViewModel> createPermissionViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework, model::PermissionKind kind);

}
