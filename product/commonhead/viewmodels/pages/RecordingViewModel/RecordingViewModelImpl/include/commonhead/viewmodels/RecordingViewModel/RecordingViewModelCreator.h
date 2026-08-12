#pragma once

#include <memory>

#include <commonhead/viewmodels/RecordingViewModel/IRecordingViewModel.h>
#include <commonhead/viewmodels/RecordingViewModel/RecordingViewModelImplExport.h>

namespace commonHead::viewModels::impl {

RECORDING_VIEW_MODEL_IMPL_API std::shared_ptr<IRecordingViewModel>
createRecordingViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
