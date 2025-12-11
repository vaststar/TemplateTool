#include "InvocationManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include "InvocationServiceLogger.h"

namespace ucf::service{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start InvocationManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
InvocationManager::InvocationManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("Create InvocationManager, address:" << this);
}

InvocationManager::~InvocationManager()
{

}

void InvocationManager::processStartupParameters()
{
    std::scoped_lock lock(mStartupParametersMutex);
    SERVICE_LOG_DEBUG("Processing startup parameters, count: " << mStartupParameters.size());
}

void InvocationManager::setStartupParameters(const std::vector<std::string>& args)
{
    SERVICE_LOG_DEBUG("set startup parameters, count: " << args.size());
    std::scoped_lock lock(mStartupParametersMutex);
    mStartupParameters = args;
}

std::vector<std::string> InvocationManager::getStartupParameters() const
{
    std::scoped_lock lock(mStartupParametersMutex);
    return mStartupParameters;
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