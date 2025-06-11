#pragma once

#include <commonHead/CommandArgumentHandler/ICommandArgumentHandler.h>

namespace commonHead{
class CommandArgumentHandler: public ICommandArgumentHandler
{
public:
    explicit CommandArgumentHandler(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual void processCommandLineArgs(const std::map<std::string, std::string>& args) override;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
};
}
