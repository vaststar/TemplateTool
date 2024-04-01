#include "CoreFramework/CoreFramework.h"

#include "ServiceCommonFile/ServiceLogger.h"
#include "CoreFramework/IService.h"


std::shared_ptr<ICoreFramework> ICoreFramework::CreateInstance()
{
    return std::make_shared<CoreFramework>();
}

CoreFramework::CoreFramework()
{
    CORE_LOG_DEBUG("create CoreFramework, address:" << this);
}

std::string CoreFramework::getName() const
{
    return "CoreFramework";
}

void CoreFramework::initServices()
{
    auto allServices = getAllServices();
    std::for_each(allServices.begin(), allServices.end(), [](std::weak_ptr<IService> service){
        if (auto servicePtr = service.lock())
        {
            servicePtr->initService();
        }
    }); 
}