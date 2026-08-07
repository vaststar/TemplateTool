#pragma once

#include <memory>

#include <ucf/Services/MiniAppService/MiniAppServiceImplExport.h>

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
MINI_APP_SERVICE_IMPL_API std::shared_ptr<ucf::service::IMiniAppService> createMiniAppService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
