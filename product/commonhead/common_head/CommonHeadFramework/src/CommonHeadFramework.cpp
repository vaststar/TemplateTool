#include "CommonHeadFramework/CommonHeadFramework.h"

#include "CommonHeadCommonFile/CommonHeadLogger.h"

CommonHeadFramework::CommonHeadFramework()
{
    COMMONHEAD_LOG_DEBUG("create CommonHeadFramework")
}

std::string CommonHeadFramework::getName() const
{
    return "CommonHeadFramework";
}