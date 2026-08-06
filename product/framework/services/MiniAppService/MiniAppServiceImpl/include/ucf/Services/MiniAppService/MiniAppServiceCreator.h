#pragma once

#include <memory>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class IMiniAppService;

}

namespace ucf::service::impl{

// Factory entry point for the MiniAppService implementation. Lives in the
// Impl library (MiniAppServiceImpl); only ServiceFactory links against it.
SERVICE_EXPORT std::shared_ptr<ucf::service::IMiniAppService> createMiniAppService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
