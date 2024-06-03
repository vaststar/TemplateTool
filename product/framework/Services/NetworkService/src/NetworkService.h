#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/NetworkService/INetworkServiceCallback.h>

namespace ucf{
class NetworkService: public virtual INetworkService,
                      public virtual ucf::utilities::NotificationHelper<INetworkServiceCallback>,
                      public std::enable_shared_from_this<NetworkService>
{
public:
    explicit NetworkService(ICoreFrameworkWPtr coreFramework);
public:
    //IService
    virtual void initService() override;
    virtual std::string getServiceName() const override;
private:
    std::weak_ptr<ICoreFramework> mCoreFrameworkWPtr;
};
}
