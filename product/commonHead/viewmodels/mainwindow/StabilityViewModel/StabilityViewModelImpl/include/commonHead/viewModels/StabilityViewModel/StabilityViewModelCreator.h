#pragma once

#include <memory>

#include <commonHead/viewModels/StabilityViewModel/IStabilityViewModel.h>
#include <commonHead/viewModels/StabilityViewModel/StabilityViewModelImplExport.h>

namespace commonHead::viewModels::impl{

STABILITY_VIEW_MODEL_IMPL_API std::shared_ptr<IStabilityViewModel> createStabilityViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
