#pragma once

#include <memory>

#include <commonHead/viewModels/CameraDirectoryViewModel/CameraDirectoryViewModelImplExport.h>
#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryViewModel.h>

namespace commonHead::viewModels::impl {

CAMERA_DIRECTORY_VIEW_MODEL_IMPL_API std::shared_ptr<ICameraDirectoryViewModel>
createCameraDirectoryViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
