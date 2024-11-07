#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>


namespace ucf::service{
class ClientInfoService final: public virtual IClientInfoService, 
                      public virtual ucf::utilities::NotificationHelper<IClientInfoServiceCallback>,
                      public ucf::framework::CoreFrameworkCallbackDefault,
                      public std::enable_shared_from_this<ClientInfoService>
{
public:
    explicit ClientInfoService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~ClientInfoService();
    ClientInfoService(const ClientInfoService&) = delete;
    ClientInfoService(ClientInfoService&&) = delete;
    ClientInfoService& operator=(const ClientInfoService&) = delete;
    ClientInfoService& operator=(ClientInfoService&&) = delete;

    //IService
    virtual std::string getServiceName() const override;

    //ICoreFrameworkCallback
    virtual void onServiceInitialized() override;
    virtual void onCoreFrameworkExit() override;

    //IClientInfoService
    virtual model::Version getApplicationVersion() const override;
protected:
    //IService
    virtual void initService() override;
private:
    void printClientInfo() const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}