#include "ServiceFactory.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/InvocationService/IInvocationService.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ImageService/IImageService.h>
#include <ucf/Services/MediaService/IMediaService.h>
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

std::shared_ptr<ucf::service::IInvocationService> ServiceFactory::createInvocationService()
{
    return ucf::service::IInvocationService::createInstance(mCoreFrameworkWPtr.lock());
}

std::shared_ptr<ucf::service::IDataWarehouseService> ServiceFactory::createDataWarehouseService()
{
    return ucf::service::IDataWarehouseService::createInstance(mCoreFrameworkWPtr.lock());
}

std::shared_ptr<ucf::service::INetworkService> ServiceFactory::createNetworkService()
{
    return ucf::service::INetworkService::createInstance(mCoreFrameworkWPtr.lock());
}

std::shared_ptr<ucf::service::IClientInfoService> ServiceFactory::createClientInfoService()
{
    return ucf::service::IClientInfoService::createInstance(mCoreFrameworkWPtr.lock());
}

std::shared_ptr<ucf::service::IContactService> ServiceFactory::createContactService()
{
    return ucf::service::IContactService::createInstance(mCoreFrameworkWPtr.lock());
}

std::shared_ptr<ucf::service::IImageService> ServiceFactory::createImageService()
{
    return ucf::service::IImageService::createInstance(mCoreFrameworkWPtr.lock());
}

std::shared_ptr<ucf::service::IMediaService> ServiceFactory::createMediaService()
{
    return ucf::service::IMediaService::createInstance(mCoreFrameworkWPtr.lock());
}
}