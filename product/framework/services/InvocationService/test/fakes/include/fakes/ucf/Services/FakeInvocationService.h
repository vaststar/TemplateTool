#pragma once

#include <trompeloeil.hpp>
#include <ucf/Services/InvocationService/IInvocationService.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IInvocationService for unit testing
 *
 * Usage:
 *   auto fakeService = std::make_shared<FakeInvocationService>();
 *   ALLOW_CALL(*fakeService, getStartupParameters()).RETURN(std::vector<std::string>{});
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
    MAKE_MOCK0(processStartupParameters, void(), override);
    MAKE_MOCK1(setStartupParameters, void(const std::vector<std::string>&), override);
    MAKE_CONST_MOCK0(getStartupParameters, std::vector<std::string>(), override);
    MAKE_MOCK1(processCommandMessage, void(const std::string&), override);

protected:
    void initService() override {}
    void deinitService() override {}
};

} // namespace ucf::service::fakes
