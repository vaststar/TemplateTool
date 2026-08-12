#include "CommonHeadFramework.h"
#include "LoggerDefine.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <commonhead/ResourceLoader/IResourceLoader.h>
#include <commonhead/ResourceLoader/ResourceLoaderCreator.h>
#include <commonhead/ServiceLocator/ServiceLocatorCreator.h>

namespace commonHead{
std::shared_ptr<ICommonHeadFramework> ICommonHeadFramework::createInstance(ucf::framework::ICoreFrameworkWPtr coreframework)
{
    return std::make_shared<CommonHeadFramework>(coreframework);
}

CommonHeadFramework::CommonHeadFramework(ucf::framework::ICoreFrameworkWPtr coreframework)
    : mCoreframeworkWPtr(coreframework)
    , mServiceLocator(impl::createServiceLocator(coreframework))
    , mResourceLoader(impl::createResourceLoader(coreframework))
{
    COMMON_HEAD_FRAMEWORK_LOG_DEBUG("create CommonHeadFramework, address:"<<this);
}

void CommonHeadFramework::initCommonheadFramework()
{
    COMMON_HEAD_FRAMEWORK_LOG_DEBUG("init CommonHeadFramework, address:"<<this);
    mResourceLoader->initResourceLoader();
}

void CommonHeadFramework::exitCommonheadFramework()
{
    COMMON_HEAD_FRAMEWORK_LOG_DEBUG("exit CommonHeadFramework, address:"<<this);
}

std::string CommonHeadFramework::getName() const
{
    return "CommonHeadFramework";
}

ucf::framework::ICoreFrameworkWPtr CommonHeadFramework::getCoreFramework() const
{
    return mCoreframeworkWPtr;
}

IResourceLoaderPtr CommonHeadFramework::getResourceLoader() const
{
    return mResourceLoader;
}

IServiceLocatorPtr CommonHeadFramework::getServiceLocator() const
{
    return mServiceLocator;
}

}
