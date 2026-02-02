#include "CrashHandlerService.h"
#include "CrashHandlerManager.h"
#include "CrashHandlerServiceLogger.h"

#include <ucf/CoreFramework/ICoreFramework.h>

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class CrashHandlerService::DataPrivate
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

CrashHandlerService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
    , mManager(std::make_unique<CrashHandlerManager>(coreFramework))
{
}

ucf::framework::ICoreFrameworkWPtr CrashHandlerService::DataPrivate::getCoreFramework() const
{
    return mCoreFramework;
}

CrashHandlerManager& CrashHandlerService::DataPrivate::getManager()
{
    return *mManager;
}

const CrashHandlerManager& CrashHandlerService::DataPrivate::getManager() const
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
////////////////////Start CrashHandlerService Logic//////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<ICrashHandlerService> ICrashHandlerService::createInstance(
    ucf::framework::ICoreFrameworkPtr coreFramework)
{
    return std::make_shared<CrashHandlerService>(coreFramework);
}

CrashHandlerService::CrashHandlerService(
    std::shared_ptr<ucf::framework::ICoreFramework> coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    CRASHHANDLER_LOG_DEBUG("CrashHandlerService created");
}

CrashHandlerService::~CrashHandlerService()
{
    CRASHHANDLER_LOG_DEBUG("CrashHandlerService destroyed");
}

void CrashHandlerService::initService()
{
    CRASHHANDLER_LOG_INFO("CrashHandlerService::initService() called");
    mDataPrivate->getManager().initialize();
}

bool CrashHandlerService::isInstalled() const
{
    return mDataPrivate->getManager().isInstalled();
}

bool CrashHandlerService::hasPendingCrashReport() const
{
    return mDataPrivate->getManager().hasPendingCrashReport();
}

std::optional<CrashInfo> CrashHandlerService::getLastCrashInfo() const
{
    return mDataPrivate->getManager().getLastCrashInfo();
}

std::vector<std::filesystem::path> CrashHandlerService::getCrashReportFiles() const
{
    return mDataPrivate->getManager().getCrashReportFiles();
}

void CrashHandlerService::clearPendingCrashReport()
{
    mDataPrivate->getManager().clearPendingCrashReport();
}

void CrashHandlerService::clearAllCrashReports()
{
    mDataPrivate->getManager().clearAllCrashReports();
}

void CrashHandlerService::forceCrashForTesting()
{
    mDataPrivate->getManager().forceCrashForTesting();
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish CrashHandlerService Logic/////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
