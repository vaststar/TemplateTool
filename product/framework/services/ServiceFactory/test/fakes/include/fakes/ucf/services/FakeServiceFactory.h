#pragma once

#include <trompeloeil.hpp>
#include <ucf/services/ServiceFactory/IServiceFactory.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IServiceFactory for unit testing
 *
 * Usage:
 *   auto fakeFactory = std::make_shared<FakeServiceFactory>();
 *   ALLOW_CALL(*fakeFactory, createServices());
 */
class FakeServiceFactory : public IServiceFactory
{
public:
    FakeServiceFactory() = default;
    ~FakeServiceFactory() override = default;

    MAKE_MOCK0(createServices, void(), override);
};

} // namespace ucf::service::fakes
