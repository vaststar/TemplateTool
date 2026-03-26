#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/NetworkService/INetworkServiceCallback.h>

namespace ucf::service{
class NetworkService final: public virtual INetworkService,
                            public virtual ucf::utilities::NotificationHelper<INetworkServiceCallback>,
                            public ucf::framework::CoreFrameworkCallbackDefault,
                            public std::enable_shared_from_this<NetworkService>
{
public:
    explicit NetworkService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    NetworkService(const NetworkService&) = delete;
    NetworkService(NetworkService&&) = delete;
    NetworkService& operator=(const NetworkService&) = delete;
    NetworkService& operator=(NetworkService&&) = delete;
    virtual ~NetworkService();
public:
    //INetworkService
    virtual network::http::INetworkHttpManagerWPtr getNetworkHttpManager() override;

    //IService
    [[nodiscard]] virtual std::string getServiceName() const override;

    //CoreFrameworkCallbackDefault
    virtual void onCoreFrameworkExit() override;
protected:
    //IService
    virtual void initService() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}
