#pragma once

#include <memory>

#include <ucf/services/ClientInfoService/ClientInfoServiceImplExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class IClientInfoService;

}

namespace ucf::service::impl{

// Factory entry point for the ClientInfoService implementation. Lives in the
// Impl library (ClientInfoServiceImpl); only ServiceFactory links against it.
CLIENT_INFO_SERVICE_IMPL_API std::shared_ptr<ucf::service::IClientInfoService> createClientInfoService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
