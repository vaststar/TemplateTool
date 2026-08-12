#include "ServiceLocator.h"

#include <utility>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/InvocationService/IInvocationService.h>
#include <ucf/services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/services/NetworkService/INetworkService.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>
#include <ucf/services/ContactService/IContactService.h>
#include <ucf/services/CameraDirectoryService/ICameraDirectoryService.h>
#include <ucf/services/ImageService/IImageService.h>
#include <ucf/services/MediaService/IMediaService.h>
#include <ucf/services/StabilityService/IStabilityService.h>
#include <ucf/services/PerformanceService/IPerformanceService.h>
#include <ucf/services/FeatureSettingsService/IFeatureSettingsService.h>
#include <ucf/services/UpgradeService/IUpgradeService.h>
#include <ucf/services/MiniAppService/IMiniAppService.h>

#include <commonHead/ServiceLocator/ServiceLocatorCreator.h>

namespace commonHead{

namespace impl{
std::shared_ptr<IServiceLocator> createServiceLocator(
    std::weak_ptr<ucf::framework::ICoreFramework> coreFramework)
{
    return std::make_shared<ServiceLocator>(std::move(coreFramework));
}
}

ServiceLocator::ServiceLocator(std::weak_ptr<ucf::framework::ICoreFramework> coreFramework)
    : mCoreFramework(std::move(coreFramework))
{
}

std::weak_ptr<ucf::service::IInvocationService> ServiceLocator::getInvocationService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IInvocationService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IDataWarehouseService> ServiceLocator::getDataWarehouseService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IDataWarehouseService>();
    }
    return {};
}

std::weak_ptr<ucf::service::INetworkService> ServiceLocator::getNetworkService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::INetworkService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IClientInfoService> ServiceLocator::getClientInfoService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IClientInfoService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IContactService> ServiceLocator::getContactService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IContactService>();
    }
    return {};
}

std::weak_ptr<ucf::service::ICameraDirectoryService> ServiceLocator::getCameraDirectoryService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::ICameraDirectoryService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IImageService> ServiceLocator::getImageService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IImageService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IMediaService> ServiceLocator::getMediaService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IMediaService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IStabilityService> ServiceLocator::getStabilityService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IStabilityService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IPerformanceService> ServiceLocator::getPerformanceService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IPerformanceService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IFeatureSettingsService> ServiceLocator::getFeatureSettingsService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IFeatureSettingsService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IUpgradeService> ServiceLocator::getUpgradeService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IUpgradeService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IMiniAppService> ServiceLocator::getMiniAppService() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        return coreFramework->getService<ucf::service::IMiniAppService>();
    }
    return {};
}

}
