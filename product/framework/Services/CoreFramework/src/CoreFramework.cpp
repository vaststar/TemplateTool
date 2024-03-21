#include "CoreFramework/CoreFramework.h"

#include "ServiceCommonFile/ServiceLogger.h"
#include "CoreFramework/IService.h"

CoreFramework::CoreFramework()
{
    CORE_LOG_DEBUG("create CoreFramework")
}

std::string CoreFramework::getName() const
{
    return "CoreFramework";
}

std::vector<std::weak_ptr<IService>> CoreFramework::getServices() const
{
    return {};
}