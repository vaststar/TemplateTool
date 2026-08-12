#pragma once

#include <memory>

#include <commonHead/viewModels/RecordingViewModel/IRecordingViewModel.h>
#include <commonHead/viewModels/RecordingViewModel/RecordingViewModelImplExport.h>

namespace commonHead::viewModels::impl {

RECORDING_VIEW_MODEL_IMPL_API std::shared_ptr<IRecordingViewModel>
createRecordingViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
