#pragma once

#include <string>
#include <memory>
#include <map>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace commonHead{
class IResourceLoader;
using IResourceLoaderPtr = std::shared_ptr<IResourceLoader>;

class IServiceLocator;
using IServiceLocatorPtr = std::shared_ptr<IServiceLocator>;

class COMMONHEAD_EXPORT ICommonHeadFramework
{
public:
    virtual std::string getName() const = 0;
    virtual void initCommonheadFramework() = 0;
    virtual void exitCommonheadFramework() = 0;
public:
    [[nodiscard]] virtual IResourceLoaderPtr getResourceLoader() const = 0;
    [[nodiscard]] virtual IServiceLocatorPtr getServiceLocator() const = 0;
public:
    static std::shared_ptr<ICommonHeadFramework> createInstance(ucf::framework::ICoreFrameworkWPtr coreframework);
};
}