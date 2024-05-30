#include "CommonHeadFramework.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

namespace commonHead{
std::shared_ptr<ICommonHeadFramework> ICommonHeadFramework::CreateInstance(ucf::ICoreFrameworkWPtr coreframework)
{
    return std::make_shared<CommonHeadFramework>(coreframework);
}

CommonHeadFramework::CommonHeadFramework(ucf::ICoreFrameworkWPtr coreframework)
{
    COMMONHEAD_LOG_DEBUG("create CommonHeadFramework, address:"<<this);
    mCoreframeworkWPtr = coreframework;
}

std::string CommonHeadFramework::getName() const
{
    return "CommonHeadFramework";
}

ucf::ICoreFrameworkWPtr CommonHeadFramework::getCoreFramework() const
{
    return mCoreframeworkWPtr;
}
}