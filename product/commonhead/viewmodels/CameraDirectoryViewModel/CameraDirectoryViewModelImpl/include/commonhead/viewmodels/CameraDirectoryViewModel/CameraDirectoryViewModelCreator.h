#pragma once

#include <memory>

#include <commonhead/viewmodels/CameraDirectoryViewModel/CameraDirectoryViewModelImplExport.h>
#include <commonhead/viewmodels/CameraDirectoryViewModel/ICameraDirectoryViewModel.h>

namespace commonHead::viewModels::impl {

CAMERA_DIRECTORY_VIEW_MODEL_IMPL_API std::shared_ptr<ICameraDirectoryViewModel>
createCameraDirectoryViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
