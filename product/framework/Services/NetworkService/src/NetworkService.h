#pragma once

#include <memory>

#include <Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/NetworkService/INetworkService.h>
#include <ucf/NetworkService/INetworkServiceCallback.h>

namespace ucf{
class NetworkService: public virtual INetworkService,
                      public virtual Utilities::NotificationHelper<INetworkServiceCallback>,
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
