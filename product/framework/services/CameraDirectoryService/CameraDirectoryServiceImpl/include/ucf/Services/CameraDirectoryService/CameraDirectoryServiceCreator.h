#pragma once

#include <memory>

#include <ucf/Services/CameraDirectoryService/CameraDirectoryServiceImplExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class ICameraDirectoryService;

}

namespace ucf::service::impl{

// Factory entry point for the CameraDirectoryService implementation. Lives in the
// Impl library (CameraDirectoryServiceImpl); only ServiceFactory links against it.
CAMERA_DIRECTORY_SERVICE_IMPL_API std::shared_ptr<ucf::service::ICameraDirectoryService> createCameraDirectoryService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
