#include "CommandArgumentHandler.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

namespace commonHead{
std::shared_ptr<ICommandArgumentHandler> ICommandArgumentHandler::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<CommandArgumentHandler>(coreFramework);
}

CommandArgumentHandler::CommandArgumentHandler(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
}
    
void CommandArgumentHandler::processCommandLineArgs(const std::map<std::string, std::string>& args)
{
    for (const auto& [key, value] : args)
    {
        COMMONHEAD_LOG_DEBUG("Command line argument: " << key << " = " << value);
    }
}
}
