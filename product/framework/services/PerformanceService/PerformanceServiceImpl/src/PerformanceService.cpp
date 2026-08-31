#include "PerformanceService.h"
#include "PerformanceServiceLogger.h"
#include "PerformanceManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/PerformanceService/PerformanceServiceCreator.h>

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
namespace impl {
std::shared_ptr<IPerformanceService> createPerformanceService(
    ucf::framework::ICoreFrameworkPtr coreFramework)
{
    return std::make_shared<PerformanceService>(coreFramework);
}
} // namespace impl

PerformanceService::PerformanceService(std::shared_ptr<ucf::framework::ICoreFramework> coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    PERFORMANCE_LOG_DEBUG("PerformanceService constructed, address: " << this);
}

PerformanceService::~PerformanceService()
{
    PERFORMANCE_LOG_DEBUG("PerformanceService destroying, address: " << this);
}

void PerformanceService::initService()
{
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
    }

    // Inject sink before starting the monitoring loop so no early events are lost.
    auto self = shared_from_this();
    mDataPrivate->getPerformanceManager().setNotificationSink(
        std::static_pointer_cast<IPerformanceNotificationSink>(self));

    mDataPrivate->getPerformanceManager().initialize();
}

void PerformanceService::deinitService()
{
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->unRegisterCallback(shared_from_this());
    }
}

void PerformanceService::onCoreFrameworkExit()
{
    PERFORMANCE_LOG_INFO("PerformanceService::onCoreFrameworkExit()");
}

// ==========================================
// Memory Monitoring
// ==========================================

MemoryInfo PerformanceService::getCurrentMemoryUsage() const
{
    return mDataPrivate->getPerformanceManager().getCurrentMemoryUsage();
}

void PerformanceService::setProcessResidentMemoryWarningThreshold(uint64_t bytes)
{
    mDataPrivate->getPerformanceManager().setProcessResidentMemoryWarningThreshold(bytes);
}

uint64_t PerformanceService::getProcessResidentMemoryWarningThreshold() const
{
    return mDataPrivate->getPerformanceManager().getProcessResidentMemoryWarningThreshold();
}

// ==========================================
// CPU Monitoring
// ==========================================

std::optional<double> PerformanceService::getProcessCpuUsagePercent() const
{
    return mDataPrivate->getPerformanceManager().getProcessCpuUsagePercent();
}

std::optional<double> PerformanceService::getSystemCpuUsagePercent() const
{
    return mDataPrivate->getPerformanceManager().getSystemCpuUsagePercent();
}

void PerformanceService::setCpuWarningThreshold(double percent)
{
    mDataPrivate->getPerformanceManager().setCpuWarningThreshold(percent);
}

double PerformanceService::getCpuWarningThreshold() const
{
    return mDataPrivate->getPerformanceManager().getCpuWarningThreshold();
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

// ==========================================
// IPerformanceNotificationSink
// ==========================================

void PerformanceService::onMemoryWarning(const MemoryInfo& memoryInfo)
{
    if (memoryInfo.processResidentBytes)
    {
        PERFORMANCE_LOG_DEBUG(
            "Process resident-memory warning triggered, usage: "
            << *memoryInfo.processResidentBytes / 1024 / 1024 << " MiB");
    }
    else
    {
        PERFORMANCE_LOG_DEBUG(
            "Process resident-memory warning triggered, usage: unavailable");
    }
    fireNotification(&IPerformanceServiceCallback::onMemoryWarning, memoryInfo);
}

void PerformanceService::onCpuWarning(double cpuPercent)
{
    PERFORMANCE_LOG_DEBUG("CPU warning triggered, usage: " << cpuPercent << " %");
    fireNotification(&IPerformanceServiceCallback::onCpuWarning, cpuPercent);
}

} // namespace ucf::service
