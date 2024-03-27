#include "CommonHeadFramework/CommonHeadFramework.h"
#include "CoreFramework/ICoreFramework.h"

#include "CommonHeadCommonFile/CommonHeadLogger.h"

std::shared_ptr<ICommonHeadFramework> ICommonHeadFramework::CreateInstance(ICoreFrameworkWPtr coreframework)
{
    return std::make_shared<CommonHeadFramework>(coreframework);
}

CommonHeadFramework::CommonHeadFramework(ICoreFrameworkWPtr coreframework)
{
    COMMONHEAD_LOG_DEBUG("create CommonHeadFramework, address:"<<this);
    mCoreframeworkWPtr = coreframework;
}

std::string CommonHeadFramework::getName() const
{
    return "CommonHeadFramework";
}

ICoreFrameworkWPtr CommonHeadFramework::getCoreFramework() const
{
    return mCoreframeworkWPtr;
}