#pragma once

#include <trompeloeil.hpp>
#include <ucf/Services/ImageService/IImageService.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IImageService for unit testing
 *
 * Usage:
 *   auto fakeService = std::make_shared<FakeImageService>();
 *   ALLOW_CALL(*fakeService, getServiceName()).RETURN("ImageService");
 */
class FakeImageService : public IImageService
{
public:
    FakeImageService() = default;
    ~FakeImageService() override = default;

    // IService
    MAKE_CONST_MOCK0(getServiceName, std::string(), override);

protected:
    void initService() override {}
};

} // namespace ucf::service::fakes
