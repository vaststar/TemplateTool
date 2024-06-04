#include "NetworkService.h"

#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>
#include <ucf/Utilities/TimeUtils/TimeUtils.h>

namespace ucf{
std::shared_ptr<INetworkService> INetworkService::CreateInstance(ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<NetworkService>(coreFramework);
}

NetworkService::NetworkService(ICoreFrameworkWPtr coreFramework)
    :mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("Create NetworkService, address:" << this);
    ucf::utilities::TimeUtils::getUTCCurrentTime();
    auto test = ucf::utilities::TimeUtils::getUTCNowInMilliseconds();
    SERVICE_LOG_DEBUG("now:" << test << ", zone:" << ucf::utilities::TimeUtils::getLocalTimeZone());

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