#include "CoreFramework.h"

#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>
#include <ucf/CoreFramework/IService.h>

namespace ucf{



/**************************************************
 * 
 * 
 * CoreFrameworkImpl
 * 
 * 
 * ************************************/

class CoreFramework::CoreFrameworkImpl
{
public:
    CoreFramework::CoreFrameworkImpl()
    {
        CORE_LOG_DEBUG("create CoreFrameworkImpl, address:" << this);
    }

};












/**************************************************
 * 
 * 
 * CoreFramework
 * 
 * 
 * ************************************/
std::shared_ptr<ICoreFramework> ICoreFramework::CreateInstance()
{
    return std::make_shared<CoreFramework>();
}

CoreFramework::CoreFramework()
    : mImpl(std::make_shared<CoreFrameworkImpl>())
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
    fireNotification(&ICoreFrameworkCallback::OnServiceInitialized);
}
}