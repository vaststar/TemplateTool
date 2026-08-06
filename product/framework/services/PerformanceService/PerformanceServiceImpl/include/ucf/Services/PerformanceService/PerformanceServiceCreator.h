#pragma once

#include <memory>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
}

namespace ucf::service{

class IPerformanceService;

}

namespace ucf::service::impl{

// Factory entry point for the PerformanceService implementation. Lives in the
// Impl library (PerformanceServiceImpl); only ServiceFactory links against it.
SERVICE_EXPORT std::shared_ptr<ucf::service::IPerformanceService> createPerformanceService(ucf::framework::ICoreFrameworkPtr coreFramework);

}
