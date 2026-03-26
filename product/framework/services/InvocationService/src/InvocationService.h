#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/InvocationService/IInvocationService.h>

namespace ucf::service{
class InvocationService final: public virtual IInvocationService, 
                               public virtual ucf::utilities::NotificationHelper<IInvocationServiceCallback>,
                               public ucf::framework::CoreFrameworkCallbackDefault,
                               public std::enable_shared_from_this<InvocationService>
{
public:
    explicit InvocationService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~InvocationService();
    InvocationService(const InvocationService&) = delete;
    InvocationService(InvocationService&&) = delete;
    InvocationService& operator=(const InvocationService&) = delete;
    InvocationService& operator=(InvocationService&&) = delete;

    //IService
    virtual std::string getServiceName() const override;

    //ICoreFrameworkCallback
    virtual void onServiceInitialized() override;
    virtual void onCoreFrameworkExit() override;

    //IInvocationService
    virtual void processStartupParameters() override;
    virtual void setStartupParameters(const std::vector<std::string>& args) override;
    virtual std::vector<std::string> getStartupParameters() const override;
    virtual void processCommandMessage(const std::string& message) override;
protected:
    //IService
    virtual void initService() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}