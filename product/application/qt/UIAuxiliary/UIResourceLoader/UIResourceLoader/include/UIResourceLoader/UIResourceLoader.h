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
    static void registerUIResourceLoader(commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
};
}