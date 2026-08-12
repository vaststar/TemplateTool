#pragma once

#include <memory>

#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>
#include <commonHead/viewModels/MediaCameraViewModel/MediaCameraViewModelImplExport.h>

namespace commonHead::viewModels::impl {

MEDIA_CAMERA_VIEW_MODEL_IMPL_API std::shared_ptr<IMediaCameraViewModel> createMediaCameraViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
