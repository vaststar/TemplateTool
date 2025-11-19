#pragma once

#include <memory>
#include <UIResourceLoader/UIResourceLoaderExport.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}
namespace UIResource{
class UIResourceLoader_EXPORT UIResourceLoader final
{
public:
    UIResourceLoader() = delete;
    UIResourceLoader(const UIResourceLoader&) = delete;
    UIResourceLoader(UIResourceLoader&&) = delete;
    UIResourceLoader& operator=(const UIResourceLoader&) = delete;
    UIResourceLoader& operator=(UIResourceLoader&&) = delete;
    ~UIResourceLoader() = delete;
public:
    static void registerUIResourceLoader(commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
};
}