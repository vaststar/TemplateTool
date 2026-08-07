#include "ServiceFactory.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/InvocationService/IInvocationService.h>
#include <ucf/Services/InvocationService/InvocationServiceCreator.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseService/DataWarehouseServiceCreator.h>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/NetworkService/NetworkServiceCreator.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/ClientInfoService/ClientInfoServiceCreator.h>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ContactService/ContactServiceCreator.h>
#include <ucf/Services/ImageService/IImageService.h>
#include <ucf/Services/ImageService/ImageServiceCreator.h>
#include <ucf/Services/MediaService/IMediaService.h>
#include <ucf/Services/StabilityService/IStabilityService.h>
#include <ucf/Services/StabilityService/StabilityServiceCreator.h>
#include <ucf/Services/PerformanceService/IPerformanceService.h>
#include <ucf/Services/PerformanceService/PerformanceServiceCreator.h>
#include <ucf/Services/FeatureSettingsService/IFeatureSettingsService.h>
#include <ucf/Services/FeatureSettingsService/FeatureSettingsServiceCreator.h>
#include <ucf/Services/CameraDirectoryService/ICameraDirectoryService.h>
#include <ucf/Services/CameraDirectoryService/CameraDirectoryServiceCreator.h>
#include <ucf/Services/UpgradeService/IUpgradeService.h>
#include <ucf/Services/UpgradeService/UpgradeServiceCreator.h>
#include <ucf/Services/MiniAppService/IMiniAppService.h>
#include <ucf/Services/MiniAppService/MiniAppServiceCreator.h>
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
