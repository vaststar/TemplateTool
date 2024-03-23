#include "CoreFramework/CoreFramework.h"

#include "ServiceCommonFile/ServiceLogger.h"
#include "CoreFramework/IService.h"


std::shared_ptr<ICoreFramework> ICoreFramework::CreateInstance()
{
    return std::make_shared<CoreFramework>();
}

CoreFramework::CoreFramework()
{
    CORE_LOG_DEBUG("create CoreFramework")
}

std::string CoreFramework::getName() const
{
    return "CoreFramework";
}
