#pragma once

#include <memory>

#include <commonhead/viewmodels/UpgradeViewModel/IUpgradeViewModel.h>
#include <commonhead/viewmodels/UpgradeViewModel/UpgradeViewModelImplExport.h>

namespace commonHead::viewModels::impl{

UPGRADE_VIEW_MODEL_IMPL_API std::shared_ptr<IUpgradeViewModel> createUpgradeViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
