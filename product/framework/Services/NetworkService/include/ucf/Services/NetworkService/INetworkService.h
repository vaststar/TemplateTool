#pragma once

#include <memory>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

// #include <ucf/CoreFramework/IService.h>
#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/Services/NetworkService/INetworkServiceCallback.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
namespace network::http{
    class INetworkHttpManager;
    using INetworkHttpManagerWPtr = std::weak_ptr<INetworkHttpManager>;
}

class SERVICE_EXPORT INetworkService: public IService, public virtual ucf::utilities::INotificationHelper<INetworkServiceCallback>
{
public:
    virtual network::http::INetworkHttpManagerWPtr getNetworkHttpManager() = 0;
public: 
    static std::shared_ptr<INetworkService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}