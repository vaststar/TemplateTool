#include "ServiceLocator.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/InvocationService/IInvocationService.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ImageService/IImageService.h>
#include <ucf/Services/MediaService/IMediaService.h>
#include <ucf/Services/StabilityService/IStabilityService.h>
#include <ucf/Services/PerformanceService/IPerformanceService.h>

namespace commonHead{
std::shared_ptr<IServiceLocator> IServiceLocator::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ServiceLocator>(coreFramework);
}

ServiceLocator::ServiceLocator(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
}

std::weak_ptr<ucf::service::IInvocationService> ServiceLocator::getInvocationService() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        return coreFramework->getService<ucf::service::IInvocationService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IDataWarehouseService> ServiceLocator::getDataWarehouseService() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        return coreFramework->getService<ucf::service::IDataWarehouseService>();
    }
    return {};
}

std::weak_ptr<ucf::service::INetworkService> ServiceLocator::getNetworkService() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        return coreFramework->getService<ucf::service::INetworkService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IClientInfoService> ServiceLocator::getClientInfoService() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        return coreFramework->getService<ucf::service::IClientInfoService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IContactService> ServiceLocator::getContactService() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        return coreFramework->getService<ucf::service::IContactService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IImageService> ServiceLocator::getImageService() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        return coreFramework->getService<ucf::service::IImageService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IMediaService> ServiceLocator::getMediaService() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        return coreFramework->getService<ucf::service::IMediaService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IStabilityService> ServiceLocator::getStabilityService() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        return coreFramework->getService<ucf::service::IStabilityService>();
    }
    return {};
}

std::weak_ptr<ucf::service::IPerformanceService> ServiceLocator::getPerformanceService() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        return coreFramework->getService<ucf::service::IPerformanceService>();
    }
    return {};
}

}
