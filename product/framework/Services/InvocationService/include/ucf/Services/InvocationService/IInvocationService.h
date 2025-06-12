#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/InvocationService/IInvocationServiceCallback.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
class SERVICE_EXPORT IInvocationService: public IService, public virtual ucf::utilities::INotificationHelper<IInvocationServiceCallback>
{
public:
    virtual ~IInvocationService() = default;
    static std::shared_ptr<IInvocationService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
public:
    virtual void processStartupParameters() = 0;
    virtual void setStartupParameters(const std::vector<std::string>& args) = 0;
    [[nodiscard]] virtual std::vector<std::string> getStartupParameters() const = 0;

    virtual void processCommandMessage(const std::string& message) = 0;
};
}