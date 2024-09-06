#include "CommonHeadFramework.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

namespace commonHead{
std::shared_ptr<ICommonHeadFramework> ICommonHeadFramework::CreateInstance(ucf::framework::ICoreFrameworkWPtr coreframework)
{
    return std::make_shared<CommonHeadFramework>(coreframework);
}

CommonHeadFramework::CommonHeadFramework(ucf::framework::ICoreFrameworkWPtr coreframework)
{
    COMMONHEAD_LOG_DEBUG("create CommonHeadFramework, address:"<<this);
    mCoreframeworkWPtr = coreframework;
}

void CommonHeadFramework::exitCommonheadFramework()
{
    COMMONHEAD_LOG_DEBUG("exit CommonHeadFramework, address:"<<this);
}

std::string CommonHeadFramework::getName() const
{
    return "CommonHeadFramework";
}

ucf::framework::ICoreFrameworkWPtr CommonHeadFramework::getCoreFramework() const
{
    return mCoreframeworkWPtr;
}
}