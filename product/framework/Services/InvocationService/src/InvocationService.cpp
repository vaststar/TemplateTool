#include "InvocationService.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include "InvocationManager.h"
#include "InvocationServiceLogger.h"

namespace ucf::service{
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
std::shared_ptr<IInvocationService> IInvocationService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<InvocationService>(coreFramework);
}

InvocationService::InvocationService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create InvocationService, address:" << this);
}

InvocationService::~InvocationService()
{
    SERVICE_LOG_DEBUG("Delete InvocationService, address:" << this);
}

void InvocationService::initService()
{
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
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

void InvocationService::setStartupParameters(const std::vector<std::string>& args)
{
    SERVICE_LOG_DEBUG("args size:" << args.size());
    mDataPrivate->getInvocationManager().setStartupParameters(args);
}

std::vector<std::string> InvocationService::getStartupParameters() const
{
    return mDataPrivate->getInvocationManager().getStartupParameters();
}

void InvocationService::processCommandMessage(const std::string& message)
{
    SERVICE_LOG_DEBUG("message:" << message);
    mDataPrivate->getInvocationManager().processCommandMessage(message);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish InvocationService Logic///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}