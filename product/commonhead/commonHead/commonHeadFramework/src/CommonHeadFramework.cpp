#include "CommonHeadFramework.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

namespace commonHead{
std::shared_ptr<ICommonHeadFramework> ICommonHeadFramework::createInstance(ucf::framework::ICoreFrameworkWPtr coreframework)
{
    return std::make_shared<CommonHeadFramework>(coreframework);
}

CommonHeadFramework::CommonHeadFramework(ucf::framework::ICoreFrameworkWPtr coreframework)
    : mCoreframeworkWPtr(coreframework)
    , mServiceLocator(IServiceLocator::createInstance(coreframework))
    , mResourceLoader(IResourceLoader::createInstance(coreframework))
{
    COMMONHEAD_LOG_DEBUG("create CommonHeadFramework, address:"<<this);
}

void CommonHeadFramework::initCommonheadFramework()
{
    COMMONHEAD_LOG_DEBUG("init CommonHeadFramework, address:"<<this);
    mResourceLoader->initResourceLoader();
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

IResourceLoaderPtr CommonHeadFramework::getResourceLoader() const
{
    return mResourceLoader;
}

IServiceLocatorPtr CommonHeadFramework::getServiceLocator() const
{
    return mServiceLocator;
}

}