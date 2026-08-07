#pragma once

#include <memory>

#include <ucf/Services/ImageService/ImageServiceImplExport.h>

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
IMAGE_SERVICE_IMPL_API std::shared_ptr<ucf::service::IImageService> createImageService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
