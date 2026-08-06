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
    auto params = getStartupParameters();
    SERVICE_LOG_DEBUG("Processing startup parameters, count: " << params.size());
}

std::vector<std::string> InvocationManager::getStartupParameters() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        return coreFramework->getStartupParameters();
    }
    return {};
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
