#pragma once

#include <memory>
#include <map>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace commonHead{
class COMMONHEAD_EXPORT ICommandArgumentHandler
{
public:
    virtual ~ICommandArgumentHandler() = default;
    virtual void processCommandLineArgs(const std::map<std::string, std::string>& args) = 0;
public:
    static std::shared_ptr<ICommandArgumentHandler> createInstance(ucf::framework::ICoreFrameworkWPtr coreframework);
};
}