#include "ServiceFactory.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/InvocationService/IInvocationService.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseService/DataWarehouseServiceCreator.h>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ImageService/IImageService.h>
#include <ucf/Services/MediaService/IMediaService.h>
#include <ucf/Services/StabilityService/IStabilityService.h>
#include <ucf/Services/PerformanceService/IPerformanceService.h>
#include <ucf/Services/FeatureSettingsService/IFeatureSettingsService.h>
#include <ucf/Services/CameraDirectoryService/ICameraDirectoryService.h>
#include <ucf/Services/UpgradeService/IUpgradeService.h>
#include <ucf/Services/MiniAppService/IMiniAppService.h>
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

void ServiceFactory::registerServices()
{
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        SERVICE_LOG_ERROR("registerServices failed: coreFramework expired");
        return;
    }

    coreFramework->registerService<ucf::service::IInvocationService>(ucf::service::IInvocationService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::IDataWarehouseService>(ucf::service::impl::createDataWarehouseService(coreFramework));
    coreFramework->registerService<ucf::service::IClientInfoService>(ucf::service::IClientInfoService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::IStabilityService>(ucf::service::IStabilityService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::IPerformanceService>(ucf::service::IPerformanceService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::INetworkService>(ucf::service::INetworkService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::IContactService>(ucf::service::IContactService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::IImageService>(ucf::service::IImageService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::IMediaService>(ucf::service::IMediaService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::IFeatureSettingsService>(ucf::service::IFeatureSettingsService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::ICameraDirectoryService>(ucf::service::ICameraDirectoryService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::IUpgradeService>(ucf::service::IUpgradeService::createInstance(coreFramework));
    coreFramework->registerService<ucf::service::IMiniAppService>(ucf::service::IMiniAppService::createInstance(coreFramework));
}
}
