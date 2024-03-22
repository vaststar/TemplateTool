#include "CommonHeadFramework/CommonHeadFramework.h"

#include "CommonHeadCommonFile/CommonHeadLogger.h"

std::shared_ptr<ICommonHeadFramework> ICommonHeadFramework::CreateInstance()
{
    return std::make_shared<CommonHeadFramework>();
}

CommonHeadFramework::CommonHeadFramework()
{
    COMMONHEAD_LOG_DEBUG("create CommonHeadFramework")
}

std::string CommonHeadFramework::getName() const
{
    return "CommonHeadFramework";
}