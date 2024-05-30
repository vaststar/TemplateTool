#pragma once

#include <string>
#include <memory>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace ucf {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace commonHead{
class COMMONHEAD_EXPORT ICommonHeadFramework
{
public:
    virtual std::string getName() const = 0;
    virtual ucf::ICoreFrameworkWPtr getCoreFramework() const = 0;
public:
    static std::shared_ptr<ICommonHeadFramework> CreateInstance(ucf::ICoreFrameworkWPtr coreframework);
};
}