#pragma once

#include <memory>

#include <commonhead/viewmodels/ScreenshotViewModel/IScreenshotViewModel.h>
#include <commonhead/viewmodels/ScreenshotViewModel/ScreenshotViewModelImplExport.h>

namespace commonHead::viewModels::impl {

SCREENSHOT_VIEW_MODEL_IMPL_API std::shared_ptr<IScreenshotViewModel>
createScreenshotViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
