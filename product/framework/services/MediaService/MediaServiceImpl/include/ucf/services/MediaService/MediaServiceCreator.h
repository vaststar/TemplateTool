#pragma once

#include <memory>

#include <ucf/services/MediaService/MediaServiceImplExport.h>

namespace ucf::framework {
class ICoreFramework;
using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {
class IMediaService;
}

namespace ucf::service::impl {

// Factory entry point for the MediaService implementation. Only consumers
// that create the service, such as ServiceFactory, link MediaServiceImpl.
MEDIA_SERVICE_IMPL_API std::shared_ptr<ucf::service::IMediaService>
createMediaService(ucf::framework::ICoreFrameworkWPtr coreFramework);

} // namespace ucf::service::impl
