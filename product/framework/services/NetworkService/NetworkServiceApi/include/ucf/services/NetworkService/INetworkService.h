#pragma once

#include <memory>

// #include <ucf/CoreFramework/IService.h>
#include <ucf/services/ServiceDeclaration/IService.h>
#include <ucf/utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/services/NetworkService/INetworkServiceCallback.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
namespace network::http{
    class INetworkHttpManager;
    using INetworkHttpManagerWPtr = std::weak_ptr<INetworkHttpManager>;
}

class INetworkService: public IService, public virtual ucf::utilities::INotificationHelper<INetworkServiceCallback>
{
public:
    INetworkService() = default;
    INetworkService(const INetworkService&) = delete;
    INetworkService(INetworkService&&) = delete;
    INetworkService& operator=(const INetworkService&) = delete;
    INetworkService& operator=(INetworkService&&) = delete;
    virtual ~INetworkService() = default;
public:
    virtual network::http::INetworkHttpManagerWPtr getNetworkHttpManager() = 0;
};
}
