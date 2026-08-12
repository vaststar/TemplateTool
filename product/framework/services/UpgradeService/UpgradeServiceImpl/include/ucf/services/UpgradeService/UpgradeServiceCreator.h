#pragma once

#include <memory>

#include <ucf/services/UpgradeService/UpgradeServiceImplExport.h>

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
UPGRADE_SERVICE_IMPL_API std::shared_ptr<ucf::service::IUpgradeService> createUpgradeService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
