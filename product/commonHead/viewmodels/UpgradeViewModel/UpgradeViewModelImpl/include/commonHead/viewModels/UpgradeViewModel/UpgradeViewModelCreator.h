#pragma once

#include <memory>

#include <commonHead/viewModels/UpgradeViewModel/IUpgradeViewModel.h>
#include <commonHead/viewModels/UpgradeViewModel/UpgradeViewModelImplExport.h>

namespace commonHead::viewModels::impl{

UPGRADE_VIEW_MODEL_IMPL_API std::shared_ptr<IUpgradeViewModel> createUpgradeViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
