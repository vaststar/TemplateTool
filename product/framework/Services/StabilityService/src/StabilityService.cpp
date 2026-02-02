#include "StabilityService.h"
#include "CrashHandlerManager.h"
#include "StabilityServiceLogger.h"

#include <ucf/CoreFramework/ICoreFramework.h>

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class StabilityService::DataPrivate
{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    CrashHandlerManager& getManager();
    const CrashHandlerManager& getManager() const;

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
    std::unique_ptr<CrashHandlerManager> mManager;
};

StabilityService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
    , mManager(std::make_unique<CrashHandlerManager>(coreFramework))
{
}

ucf::framework::ICoreFrameworkWPtr StabilityService::DataPrivate::getCoreFramework() const
{
    return mCoreFramework;
}

CrashHandlerManager& StabilityService::DataPrivate::getManager()
{
    return *mManager;
}

const CrashHandlerManager& StabilityService::DataPrivate::getManager() const
{
    return *mManager;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start StabilityService Logic/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IStabilityService> IStabilityService::createInstance(
    ucf::framework::ICoreFrameworkPtr coreFramework)
{
    return std::make_shared<StabilityService>(coreFramework);
}

StabilityService::StabilityService(
    std::shared_ptr<ucf::framework::ICoreFramework> coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    CRASHHANDLER_LOG_DEBUG("StabilityService created");
}

StabilityService::~StabilityService()
{
    CRASHHANDLER_LOG_DEBUG("StabilityService destroyed");
}

void StabilityService::initService()
{
    CRASHHANDLER_LOG_INFO("StabilityService::initService() called");
    mDataPrivate->getManager().initialize();
}

bool StabilityService::isInstalled() const
{
    return mDataPrivate->getManager().isInstalled();
}

bool StabilityService::hasPendingCrashReport() const
{
    return mDataPrivate->getManager().hasPendingCrashReport();
}

std::optional<CrashInfo> StabilityService::getLastCrashInfo() const
{
    return mDataPrivate->getManager().getLastCrashInfo();
}

std::vector<std::filesystem::path> StabilityService::getCrashReportFiles() const
{
    return mDataPrivate->getManager().getCrashReportFiles();
}

void StabilityService::clearPendingCrashReport()
{
    mDataPrivate->getManager().clearPendingCrashReport();
}

void StabilityService::clearAllCrashReports()
{
    mDataPrivate->getManager().clearAllCrashReports();
}

void StabilityService::forceCrashForTesting()
{
    mDataPrivate->getManager().forceCrashForTesting();
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish StabilityService Logic////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
