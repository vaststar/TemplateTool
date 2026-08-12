#pragma once

#include <memory>

#include <commonhead/viewmodels/StabilityViewModel/IStabilityViewModel.h>
#include <commonhead/viewmodels/StabilityViewModel/StabilityViewModelImplExport.h>

namespace commonHead::viewModels::impl{

STABILITY_VIEW_MODEL_IMPL_API std::shared_ptr<IStabilityViewModel> createStabilityViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
