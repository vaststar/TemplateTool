#include "ClientInfoService.h"



namespace ucf::service{
std::shared_ptr<IClientInfoService> IClientInfoService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ClientInfoService>(coreFramework);
}
}