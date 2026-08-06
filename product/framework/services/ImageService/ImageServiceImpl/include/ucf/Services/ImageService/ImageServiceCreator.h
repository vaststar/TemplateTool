#pragma once

#include <memory>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class IImageService;

}

namespace ucf::service::impl{

// Factory entry point for the ImageService implementation. Lives in the
// Impl library (ImageServiceImpl); only ServiceFactory links against it.
SERVICE_EXPORT std::shared_ptr<ucf::service::IImageService> createImageService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
