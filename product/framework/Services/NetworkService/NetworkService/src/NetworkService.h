#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/NetworkService/INetworkServiceCallback.h>

namespace ucf{
class NetworkService: public virtual INetworkService,
                      public virtual ucf::utilities::NotificationHelper<INetworkServiceCallback>,
                      public CoreFrameworkCallbackDefault,
                      public std::enable_shared_from_this<NetworkService>
{
public:
    explicit NetworkService(ICoreFrameworkWPtr coreFramework);
    virtual ~NetworkService();
public:
    //INetworkService
    virtual network::http::INetworkHttpManagerWPtr getNetworkHttpManager() override;

    //IService
    virtual void initService() override;
    virtual std::string getServiceName() const override;

    //CoreFrameworkCallbackDefault
    virtual void onCoreFrameworkExit() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}
