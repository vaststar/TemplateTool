#include <ucf/services/ServiceDeclaration/IService.h>

#include "LoggerDefine.h"

namespace ucf::service {

void IService::initComponent()
{
    std::call_once(mInitFlag, [this]() {
        const auto serviceName = getServiceName();
        const auto* objectAddress = dynamic_cast<const void*>(this);

        SERVICE_DECLARATION_LOG_DEBUG(
            serviceName
            << " initialization started, address: "
            << objectAddress);

        initService();

        SERVICE_DECLARATION_LOG_DEBUG(
            serviceName
            << " initialization finished, address: "
            << objectAddress);
    });
}

void IService::deinitComponent()
{
    std::call_once(mDeinitFlag, [this]() {
        const auto serviceName = getServiceName();
        const auto* objectAddress = dynamic_cast<const void*>(this);

        SERVICE_DECLARATION_LOG_DEBUG(
            serviceName
            << " deinitialization started, address: "
            << objectAddress);

        deinitService();

        SERVICE_DECLARATION_LOG_DEBUG(
            serviceName
            << " deinitialization finished, address: "
            << objectAddress);
    });
}

std::vector<ServiceDependency> IService::dependencies() const
{
    return {};
}

} // namespace ucf::service
