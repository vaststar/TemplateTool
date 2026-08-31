#pragma once

#include <ucf/utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>

#include <ucf/services/PerformanceService/IPerformanceService.h>
#include <ucf/services/PerformanceService/PerformanceServiceImplExport.h>

#include "PerformanceNotificationSink.h"

#include <memory>

namespace ucf::framework {
    class ICoreFramework;
}

namespace ucf::service {

class PERFORMANCE_SERVICE_IMPL_API PerformanceService : public virtual IPerformanceService,
                           public virtual ucf::utilities::NotificationHelper<IPerformanceServiceCallback>,
                           public IPerformanceNotificationSink,
                           public ucf::framework::CoreFrameworkCallbackDefault,
                           public std::enable_shared_from_this<PerformanceService>
{
public:
    explicit PerformanceService(std::shared_ptr<ucf::framework::ICoreFramework> coreFramework);
    ~PerformanceService() override;

    // IService
    [[nodiscard]] std::string getServiceName() const override { return "PerformanceService"; }

    // Memory Monitoring
    [[nodiscard]] MemoryInfo getCurrentMemoryUsage() const override;
    void setProcessResidentMemoryWarningThreshold(uint64_t bytes) override;
    [[nodiscard]] uint64_t getProcessResidentMemoryWarningThreshold() const override;

    // CPU Monitoring
    [[nodiscard]] std::optional<double> getProcessCpuUsagePercent() const override;
    [[nodiscard]] std::optional<double> getSystemCpuUsagePercent() const override;
    void setCpuWarningThreshold(double percent) override;
    [[nodiscard]] double getCpuWarningThreshold() const override;

    // Timing
    [[nodiscard]] TimingToken beginTiming(const std::string& operationName) override;
    void endTiming(const TimingToken& token) override;
    [[nodiscard]] std::optional<TimingStats> getTimingStats(
        const std::string& operationName) const override;
    [[nodiscard]] std::vector<TimingStats> getAllTimingStats() const override;
    void resetTimingStats() override;

    // Snapshot & Export
    [[nodiscard]] PerformanceSnapshot takeSnapshot() const override;
    [[nodiscard]] std::string exportReportAsJson() const override;
    void exportReportToFile(const std::filesystem::path& path) const override;

    // Callback - uses NotificationHelper::registerCallback/unregisterCallback

    // CoreFrameworkCallbackDefault
    void onCoreFrameworkExit() override;

protected:
    void initService() override;
    void deinitService() override;

    // IPerformanceNotificationSink — translates internal threshold events to outward callbacks
    void onMemoryWarning(const MemoryInfo& memoryInfo) override;
    void onCpuWarning(double cpuPercent) override;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

} // namespace ucf::service
