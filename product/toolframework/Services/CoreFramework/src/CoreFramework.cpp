#include "CoreFramework.h"

#include "ServiceLogger.h"
#include "IService.h"

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