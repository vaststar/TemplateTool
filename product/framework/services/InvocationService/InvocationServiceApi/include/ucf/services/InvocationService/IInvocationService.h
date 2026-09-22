#pragma once

#include <optional>
#include <string>

#include <ucf/utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/services/ServiceDeclaration/IService.h>
#include <ucf/services/InvocationService/IInvocationServiceCallback.h>
#include <ucf/services/InvocationService/StartupContext.h>

namespace ucf::service{
class IInvocationService: public IService, public virtual ucf::utilities::INotificationHelper<IInvocationServiceCallback>
{
public:
    IInvocationService() = default;
    IInvocationService(const IInvocationService&) = delete;
    IInvocationService(IInvocationService&&) = delete;
    IInvocationService& operator=(const IInvocationService&) = delete;
    IInvocationService& operator=(IInvocationService&&) = delete;
    virtual ~IInvocationService() = default;
public:
    virtual void processStartupParameters(StartupContext context) = 0;
    [[nodiscard]] virtual std::optional<StartupContext> getStartupContext() const = 0;

    virtual void processCommandMessage(const std::string& message) = 0;
};
}
