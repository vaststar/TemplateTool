#pragma once

#include <memory>

#include <ucf/Services/NetworkService/NetworkServiceImplExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class INetworkService;

}

namespace ucf::service::impl{

// Factory entry point for the NetworkService implementation. Lives in the
// Impl library (NetworkServiceImpl); only ServiceFactory links against it.
NETWORK_SERVICE_IMPL_API std::shared_ptr<ucf::service::INetworkService> createNetworkService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
