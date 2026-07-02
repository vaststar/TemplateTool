#pragma once

#include <trompeloeil.hpp>
#include <ucf/Services/ServiceFactory/IServiceFactory.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IServiceFactory for unit testing
 *
 * Usage:
 *   auto fakeFactory = std::make_shared<FakeServiceFactory>();
 *   ALLOW_CALL(*fakeFactory, createNetworkService()).RETURN(nullptr);
 */
class FakeServiceFactory : public IServiceFactory
{
public:
    FakeServiceFactory() = default;
    ~FakeServiceFactory() override = default;

    MAKE_MOCK0(createInvocationService, std::shared_ptr<IInvocationService>(), override);
    MAKE_MOCK0(createDataWarehouseService, std::shared_ptr<IDataWarehouseService>(), override);
    MAKE_MOCK0(createNetworkService, std::shared_ptr<INetworkService>(), override);
    MAKE_MOCK0(createClientInfoService, std::shared_ptr<IClientInfoService>(), override);
    MAKE_MOCK0(createContactService, std::shared_ptr<IContactService>(), override);
    MAKE_MOCK0(createImageService, std::shared_ptr<IImageService>(), override);
    MAKE_MOCK0(createMediaService, std::shared_ptr<IMediaService>(), override);
    MAKE_MOCK0(createStabilityService, std::shared_ptr<IStabilityService>(), override);
    MAKE_MOCK0(createPerformanceService, std::shared_ptr<IPerformanceService>(), override);
    MAKE_MOCK0(createFeatureSettingsService, std::shared_ptr<IFeatureSettingsService>(), override);
    MAKE_MOCK0(createCameraDirectoryService, std::shared_ptr<ICameraDirectoryService>(), override);
    MAKE_MOCK0(createUpgradeService, std::shared_ptr<IUpgradeService>(), override);
    MAKE_MOCK0(createMiniAppService, std::shared_ptr<IMiniAppService>(), override);
};

} // namespace ucf::service::fakes
