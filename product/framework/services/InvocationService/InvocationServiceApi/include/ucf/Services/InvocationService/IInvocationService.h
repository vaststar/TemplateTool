#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/InvocationService/IInvocationServiceCallback.h>

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
    virtual void processStartupParameters() = 0;
    [[nodiscard]] virtual std::vector<std::string> getStartupParameters() const = 0;

    virtual void processCommandMessage(const std::string& message) = 0;
};
}
