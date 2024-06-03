#include "NetworkService.h"

#include <ucf/ServiceCommonFile/ServiceLogger.h>

namespace ucf{
std::shared_ptr<INetworkService> INetworkService::CreateInstance(ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<NetworkService>(coreFramework);
}

NetworkService::NetworkService(ICoreFrameworkWPtr coreFramework)
    :mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("Create NetworkService, address:" << this);
}

void NetworkService::initService()
{
    SERVICE_LOG_DEBUG("init NetworkService, address:" << this);
}

std::string NetworkService::getServiceName() const
{
    return "NetworkService";
}
}