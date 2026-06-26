#pragma once

#include <trompeloeil.hpp>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of INetworkService for unit testing
 *
 * Usage:
 *   auto fakeService = std::make_shared<FakeNetworkService>();
 *   ALLOW_CALL(*fakeService, getNetworkHttpManager()).RETURN(...);
 */
class FakeNetworkService
    : public INetworkService
    , public ucf::utilities::NotificationHelper<INetworkServiceCallback>
{
public:
    FakeNetworkService() = default;
    ~FakeNetworkService() override = default;

    // IService
    MAKE_CONST_MOCK0(getServiceName, std::string(), override);

    // INetworkService
    MAKE_MOCK0(getNetworkHttpManager, network::http::INetworkHttpManagerWPtr(), override);

protected:
    void initService() override {}
};

} // namespace ucf::service::fakes
