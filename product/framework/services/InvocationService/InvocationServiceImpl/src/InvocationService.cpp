#include "InvocationService.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/InvocationService/InvocationServiceCreator.h>

#include "InvocationManager.h"
#include "InvocationServiceLogger.h"

namespace ucf::service::impl{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class InvocationService::DataPrivate{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;

    InvocationManager& getInvocationManager();
    const InvocationManager& getInvocationManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::unique_ptr<InvocationManager> mInvocationManager;
};

InvocationService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mInvocationManager(std::make_unique<InvocationManager>(coreFramework))
{
}

ucf::framework::ICoreFrameworkWPtr InvocationService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}


InvocationManager& InvocationService::DataPrivate::getInvocationManager()
{
    return *mInvocationManager;
}

const InvocationManager& InvocationService::DataPrivate::getInvocationManager() const
{
    return *mInvocationManager;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start InvocationService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ucf::service::IInvocationService> createInvocationService(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<InvocationService>(coreFramework);
}

InvocationService::InvocationService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_DEBUG("InvocationService constructed, address: " << this);
}

InvocationService::~InvocationService()
{
    SERVICE_LOG_DEBUG("InvocationService destroying, address: " << this);
}

void InvocationService::initService()
{
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
    }
}

void InvocationService::deinitService()
{
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->unRegisterCallback(shared_from_this());
    }
}

std::string InvocationService::getServiceName() const
{
    return "InvocationService";
}

void InvocationService::onServiceInitialized()
{
    SERVICE_LOG_DEBUG("");
}

void InvocationService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("");
}

void InvocationService::processStartupParameters()
{
    SERVICE_LOG_DEBUG("");
    mDataPrivate->getInvocationManager().processStartupParameters();
}

std::vector<std::string> InvocationService::getStartupParameters() const
{
    return mDataPrivate->getInvocationManager().getStartupParameters();
}

void InvocationService::processCommandMessage(const std::string& message)
{
    SERVICE_LOG_DEBUG("message:" << message);
    mDataPrivate->getInvocationManager().processCommandMessage(message);
    fireNotification(&IInvocationServiceCallback::onCommandMessageReceived, message);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish InvocationService Logic///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
