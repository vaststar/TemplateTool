#include "StabilityService.h"
#include "crash/CrashManager.h"
#include "hang/HangManager.h"
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
    
    CrashManager& getCrashManager();
    const CrashManager& getCrashManager() const;
    
    HangManager& getHangManager();
    const HangManager& getHangManager() const;

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
    std::unique_ptr<CrashManager> mCrashManager;
    std::unique_ptr<HangManager> mHangManager;
};

StabilityService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
    , mCrashManager(std::make_unique<CrashManager>(coreFramework))
    , mHangManager(std::make_unique<HangManager>(coreFramework))
{
}

ucf::framework::ICoreFrameworkWPtr StabilityService::DataPrivate::getCoreFramework() const
{
    return mCoreFramework;
}

CrashManager& StabilityService::DataPrivate::getCrashManager()
{
    return *mCrashManager;
}

const CrashManager& StabilityService::DataPrivate::getCrashManager() const
{
    return *mCrashManager;
}

HangManager& StabilityService::DataPrivate::getHangManager()
{
    return *mHangManager;
}

const HangManager& StabilityService::DataPrivate::getHangManager() const
{
    return *mHangManager;
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
    
    // Initialize crash manager
    mDataPrivate->getCrashManager().initialize();
    
    // Initialize hang detection
    mDataPrivate->getHangManager().initialize();
}

// ==========================================
// Crash Report Implementation
// ==========================================

bool StabilityService::isCrashHandlerInstalled() const
{
    return mDataPrivate->getCrashManager().isInstalled();
}

bool StabilityService::hasPendingCrashReport() const
{
    return mDataPrivate->getCrashManager().hasPendingCrashReport();
}

std::optional<CrashInfo> StabilityService::getLastCrashInfo() const
{
    return mDataPrivate->getCrashManager().getLastCrashInfo();
}

std::vector<std::filesystem::path> StabilityService::getCrashReportFiles() const
{
    return mDataPrivate->getCrashManager().getCrashReportFiles();
}

void StabilityService::clearPendingCrashReport()
{
    mDataPrivate->getCrashManager().clearPendingCrashReport();
}

void StabilityService::clearAllCrashReports()
{
    mDataPrivate->getCrashManager().clearAllCrashReports();
}

void StabilityService::forceCrashForTesting()
{
    mDataPrivate->getCrashManager().forceCrashForTesting();
}

// ==========================================
// Hang Detection Implementation
// ==========================================

void StabilityService::reportHeartbeat()
{
    mDataPrivate->getHangManager().reportHeartbeat();
}

bool StabilityService::isHangDetectionEnabled() const
{
    return mDataPrivate->getHangManager().isEnabled();
}

bool StabilityService::hasPendingHangReport() const
{
    return mDataPrivate->getHangManager().hasPendingHangReport();
}

std::optional<HangInfo> StabilityService::getLastHangInfo() const
{
    return mDataPrivate->getHangManager().getLastHangInfo();
}

std::vector<std::filesystem::path> StabilityService::getHangReportFiles() const
{
    return mDataPrivate->getHangManager().getHangReportFiles();
}

void StabilityService::clearPendingHangReport()
{
    mDataPrivate->getHangManager().clearPendingHangReport();
}

void StabilityService::clearAllHangReports()
{
    mDataPrivate->getHangManager().clearAllHangReports();
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish StabilityService Logic////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
