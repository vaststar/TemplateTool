#pragma once

#include <memory>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
}

namespace ucf::service{

class IStabilityService;

}

namespace ucf::service::impl{

// Factory entry point for the StabilityService implementation. Lives in the
// Impl library (StabilityServiceImpl); only ServiceFactory links against it.
SERVICE_EXPORT std::shared_ptr<ucf::service::IStabilityService> createStabilityService(ucf::framework::ICoreFrameworkPtr coreFramework);

}
