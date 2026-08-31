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
    COMMON_HEAD_FRAMEWORK_LOG_DEBUG("CommonHeadFramework constructed, address: " << this);
}

CommonHeadFramework::~CommonHeadFramework()
{
    COMMON_HEAD_FRAMEWORK_LOG_DEBUG("CommonHeadFramework destroying, address: " << this);
}

void CommonHeadFramework::initCommonheadFramework()
{
    COMMON_HEAD_FRAMEWORK_LOG_DEBUG(
        "CommonHeadFramework initialization started, address: "
        << this);

    mResourceLoader->initResourceLoader();

    COMMON_HEAD_FRAMEWORK_LOG_DEBUG(
        "CommonHeadFramework initialization finished, address: "
        << this);
}

void CommonHeadFramework::exitCommonheadFramework()
{
    COMMON_HEAD_FRAMEWORK_LOG_DEBUG(
        "CommonHeadFramework shutdown started, address: "
        << this);

    COMMON_HEAD_FRAMEWORK_LOG_DEBUG(
        "CommonHeadFramework shutdown finished, address: "
        << this);
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
