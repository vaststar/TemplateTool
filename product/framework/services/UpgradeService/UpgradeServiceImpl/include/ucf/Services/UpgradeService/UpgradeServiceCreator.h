#pragma once

#include <memory>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class IUpgradeService;

}

namespace ucf::service::impl{

// Factory entry point for the UpgradeService implementation. Lives in the
// Impl library (UpgradeServiceImpl); only ServiceFactory links against it.
SERVICE_EXPORT std::shared_ptr<ucf::service::IUpgradeService> createUpgradeService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
