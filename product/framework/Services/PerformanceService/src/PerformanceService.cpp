#include "PerformanceService.h"
#include "PerformanceServiceLogger.h"
#include "PerformanceManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class PerformanceService::DataPrivate
{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    PerformanceManager& getPerformanceManager();
    const PerformanceManager& getPerformanceManager() const;

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::unique_ptr<PerformanceManager> mPerformanceManager;
};

PerformanceService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mPerformanceManager(std::make_unique<PerformanceManager>(coreFramework))
{
}

ucf::framework::ICoreFrameworkWPtr PerformanceService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

PerformanceManager& PerformanceService::DataPrivate::getPerformanceManager()
{
    return *mPerformanceManager;
}

const PerformanceManager& PerformanceService::DataPrivate::getPerformanceManager() const
{
    return *mPerformanceManager;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start PerformanceService Logic///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

// Factory method
std::shared_ptr<IPerformanceService> IPerformanceService::createInstance(
    ucf::framework::ICoreFrameworkPtr coreFramework)
{
    return std::make_shared<PerformanceService>(coreFramework);
}

PerformanceService::PerformanceService(std::shared_ptr<ucf::framework::ICoreFramework> coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    PERFORMANCE_LOG_DEBUG("Create PerformanceService, address:" << this);
}

PerformanceService::~PerformanceService()
{
    PERFORMANCE_LOG_DEBUG("Delete PerformanceService, address:" << this);
}

void PerformanceService::initService()
{
    mDataPrivate->getPerformanceManager().initialize();
    PERFORMANCE_LOG_INFO("PerformanceService initialized");
}

// ==========================================
// Memory Monitoring
// ==========================================

MemoryInfo PerformanceService::getCurrentMemoryUsage() const
{
    return mDataPrivate->getPerformanceManager().getCurrentMemoryUsage();
}

void PerformanceService::setMemoryWarningThreshold(uint64_t bytes)
{
    mDataPrivate->getPerformanceManager().setMemoryWarningThreshold(bytes);
}

uint64_t PerformanceService::getMemoryWarningThreshold() const
{
    return mDataPrivate->getPerformanceManager().getMemoryWarningThreshold();
}

// ==========================================
// CPU Monitoring
// ==========================================

double PerformanceService::getCPUUsage() const
{
    return mDataPrivate->getPerformanceManager().getCPUUsage();
}

// ==========================================
// Timing
// ==========================================

TimingToken PerformanceService::beginTiming(const std::string& operationName)
{
    return mDataPrivate->getPerformanceManager().beginTiming(operationName);
}

void PerformanceService::endTiming(const TimingToken& token)
{
    mDataPrivate->getPerformanceManager().endTiming(token);
}

TimingStats PerformanceService::getTimingStats(const std::string& operationName) const
{
    return mDataPrivate->getPerformanceManager().getTimingStats(operationName);
}

std::vector<TimingStats> PerformanceService::getAllTimingStats() const
{
    return mDataPrivate->getPerformanceManager().getAllTimingStats();
}

void PerformanceService::resetTimingStats()
{
    mDataPrivate->getPerformanceManager().resetTimingStats();
}

// ==========================================
// Snapshot & Export
// ==========================================

PerformanceSnapshot PerformanceService::takeSnapshot() const
{
    return mDataPrivate->getPerformanceManager().takeSnapshot();
}

std::string PerformanceService::exportReportAsJson() const
{
    return mDataPrivate->getPerformanceManager().exportReportAsJson();
}

void PerformanceService::exportReportToFile(const std::filesystem::path& path) const
{
    mDataPrivate->getPerformanceManager().exportReportToFile(path);
}

} // namespace ucf::service
