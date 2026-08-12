#pragma once

#include <memory>

#include <ucf/services/PerformanceService/PerformanceServiceImplExport.h>

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
PERFORMANCE_SERVICE_IMPL_API std::shared_ptr<ucf::service::IPerformanceService> createPerformanceService(ucf::framework::ICoreFrameworkPtr coreFramework);

}
