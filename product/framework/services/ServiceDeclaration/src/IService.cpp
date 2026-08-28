#include <ucf/services/ServiceDeclaration/IService.h>

#include "LoggerDefine.h"

namespace ucf::service {

void IService::initComponent()
{
    std::call_once(mInitFlag, [this]() {
        const auto serviceName = getServiceName();

        SERVICE_DECLARATION_LOG_DEBUG(
            serviceName << " initialization started, address: " << this);

        initService();

        SERVICE_DECLARATION_LOG_DEBUG(
            serviceName << " initialization finished, address: " << this);
    });
}

void IService::deinitComponent()
{
    std::call_once(mDeinitFlag, [this]() {
        const auto serviceName = getServiceName();

        SERVICE_DECLARATION_LOG_DEBUG(
            serviceName << " deinitialization started, address: " << this);

        deinitService();

        SERVICE_DECLARATION_LOG_DEBUG(
            serviceName << " deinitialization finished, address: " << this);
    });
}

std::vector<ServiceDependency> IService::dependencies() const
{
    return {};
}

} // namespace ucf::service
