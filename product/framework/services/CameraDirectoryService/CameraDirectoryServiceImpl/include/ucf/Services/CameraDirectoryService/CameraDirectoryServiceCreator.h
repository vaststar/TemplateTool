#pragma once

#include <memory>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

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
SERVICE_EXPORT std::shared_ptr<ucf::service::ICameraDirectoryService> createCameraDirectoryService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
