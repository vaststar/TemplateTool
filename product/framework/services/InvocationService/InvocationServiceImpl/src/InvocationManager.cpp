#include "InvocationManager.h"

#include <utility>

#include <ucf/CoreFramework/ICoreFramework.h>

#include "InvocationServiceLogger.h"

namespace ucf::service::impl{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start InvocationManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
InvocationManager::InvocationManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("InvocationManager constructed, address: " << this);
}

InvocationManager::~InvocationManager()
{
    SERVICE_LOG_DEBUG("InvocationManager destroying, address: " << this);
}

void InvocationManager::processStartupParameters(StartupContext context)
{
    const auto parameterCount = context.commandLineArguments.size();

    {
        std::scoped_lock lock(mStartupContextMutex);
        if (mStartupContext.has_value())
        {
            SERVICE_LOG_WARN("Startup parameter processing skipped: context was already supplied");
            return;
        }

        mStartupContext.emplace(std::move(context));
    }

    SERVICE_LOG_DEBUG("Startup parameters stored, count: " << parameterCount);
}

std::optional<StartupContext> InvocationManager::getStartupContext() const
{
    std::scoped_lock lock(mStartupContextMutex);
    return mStartupContext;
}

void InvocationManager::processCommandMessage(const std::string& message)
{
    {
        std::scoped_lock lock(mCommandMessagesMutex);
        mCommandMessages.push_back(message);
        SERVICE_LOG_DEBUG("Added command message to history, total count: " << mCommandMessages.size());
    }
    SERVICE_LOG_DEBUG("Processing command message: " << message);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start InvocationManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
