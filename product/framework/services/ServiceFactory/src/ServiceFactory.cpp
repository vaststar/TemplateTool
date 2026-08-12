#include "ServiceFactory.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/InvocationService/IInvocationService.h>
#include <ucf/services/InvocationService/InvocationServiceCreator.h>
#include <ucf/services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/services/DataWarehouseService/DataWarehouseServiceCreator.h>
#include <ucf/services/NetworkService/INetworkService.h>
#include <ucf/services/NetworkService/NetworkServiceCreator.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>
#include <ucf/services/ClientInfoService/ClientInfoServiceCreator.h>
#include <ucf/services/ContactService/IContactService.h>
#include <ucf/services/ContactService/ContactServiceCreator.h>
#include <ucf/services/ImageService/IImageService.h>
#include <ucf/services/ImageService/ImageServiceCreator.h>
#include <ucf/services/MediaService/IMediaService.h>
#include <ucf/services/StabilityService/IStabilityService.h>
#include <ucf/services/StabilityService/StabilityServiceCreator.h>
#include <ucf/services/PerformanceService/IPerformanceService.h>
#include <ucf/services/PerformanceService/PerformanceServiceCreator.h>
#include <ucf/services/FeatureSettingsService/IFeatureSettingsService.h>
#include <ucf/services/FeatureSettingsService/FeatureSettingsServiceCreator.h>
#include <ucf/services/CameraDirectoryService/ICameraDirectoryService.h>
#include <ucf/services/CameraDirectoryService/CameraDirectoryServiceCreator.h>
#include <ucf/services/UpgradeService/IUpgradeService.h>
#include <ucf/services/UpgradeService/UpgradeServiceCreator.h>
#include <ucf/services/MiniAppService/IMiniAppService.h>
#include <ucf/services/MiniAppService/MiniAppServiceCreator.h>
#include "ServiceFactoryLogger.h"

namespace ucf::service{
std::shared_ptr<IServiceFactory> IServiceFactory::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ServiceFactory>(coreFramework);
}

ServiceFactory::ServiceFactory(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_INFO("create ServiceFactory, address:" << this);
}

ServiceFactory::~ServiceFactory()
{
    SERVICE_LOG_INFO("delete ServiceFactory, address:" << this);
}

void ServiceFactory::createServices()
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        SERVICE_LOG_ERROR("createServices failed: coreFramework expired");
        return;
    }

    coreFramework->registerService<ucf::service::IInvocationService>(ucf::service::impl::createInvocationService(coreFramework));
    coreFramework->registerService<ucf::service::IDataWarehouseService>(ucf::service::impl::createDataWarehouseService(coreFramework));
    coreFramework->registerService<ucf::service::IClientInfoService>(ucf::service::impl::createClientInfoService(coreFramework));
    coreFramework->registerService<ucf::service::IStabilityService>(ucf::service::impl::createStabilityService(coreFramework));
    coreFramework->registerService<ucf::service::IPerformanceService>(ucf::service::impl::createPerformanceService(coreFramework));
    coreFramework->registerService<ucf::service::INetworkService>(ucf::service::impl::createNetworkService(coreFramework));
    coreFramework->registerService<ucf::service::IContactService>(ucf::service::impl::createContactService(coreFramework));
    coreFramework->registerService<ucf::service::IImageService>(ucf::service::impl::createImageService(coreFramework));
    coreFramework->registerService<ucf::service::IMediaService>(ucf::service::IMediaService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::IFeatureSettingsService>(ucf::service::impl::createFeatureSettingsService(coreFramework));
    coreFramework->registerService<ucf::service::ICameraDirectoryService>(ucf::service::impl::createCameraDirectoryService(coreFramework));
    coreFramework->registerService<ucf::service::IUpgradeService>(ucf::service::impl::createUpgradeService(coreFramework));
    coreFramework->registerService<ucf::service::IMiniAppService>(ucf::service::impl::createMiniAppService(coreFramework));
}
}
