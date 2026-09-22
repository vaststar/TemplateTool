#pragma once

#include <optional>

#include <trompeloeil.hpp>
#include <ucf/services/InvocationService/IInvocationService.h>
#include <ucf/utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IInvocationService for unit testing
 *
 * Usage:
 *   auto fakeService = std::make_shared<FakeInvocationService>();
 *   ALLOW_CALL(*fakeService, getStartupContext()).RETURN(std::nullopt);
 */
class FakeInvocationService
    : public IInvocationService
    , public ucf::utilities::NotificationHelper<IInvocationServiceCallback>
{
public:
    FakeInvocationService() = default;
    ~FakeInvocationService() override = default;

    // IService
    MAKE_CONST_MOCK0(getServiceName, std::string(), override);

    // IInvocationService
    MAKE_MOCK1(processStartupParameters, void(StartupContext), override);
    MAKE_CONST_MOCK0(getStartupContext, std::optional<StartupContext>(), override);
    MAKE_MOCK1(processCommandMessage, void(const std::string&), override);

protected:
    void initService() override {}
    void deinitService() override {}
};

} // namespace ucf::service::fakes
