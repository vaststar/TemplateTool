#pragma once

#include <memory>

#include <ucf/services/StabilityService/StabilityServiceImplExport.h>

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
STABILITY_SERVICE_IMPL_API std::shared_ptr<ucf::service::IStabilityService> createStabilityService(ucf::framework::ICoreFrameworkPtr coreFramework);

}
