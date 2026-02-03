#pragma once

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/Services/PerformanceService/IPerformanceService.h>

#include <memory>

namespace ucf::framework {
    class ICoreFramework;
}

namespace ucf::service {

class PerformanceService : public virtual IPerformanceService,
                           public virtual ucf::utilities::NotificationHelper<IPerformanceServiceCallback>,
                           public std::enable_shared_from_this<PerformanceService>
{
public:
    explicit PerformanceService(std::shared_ptr<ucf::framework::ICoreFramework> coreFramework);
    ~PerformanceService() override;

    // IService
    [[nodiscard]] std::string getServiceName() const override { return "PerformanceService"; }

    // Memory Monitoring
    [[nodiscard]] MemoryInfo getCurrentMemoryUsage() const override;
    void setMemoryWarningThreshold(uint64_t bytes) override;
    [[nodiscard]] uint64_t getMemoryWarningThreshold() const override;

    // CPU Monitoring
    [[nodiscard]] double getCPUUsage() const override;

    // Timing
    [[nodiscard]] TimingToken beginTiming(const std::string& operationName) override;
    void endTiming(const TimingToken& token) override;
    [[nodiscard]] TimingStats getTimingStats(const std::string& operationName) const override;
    [[nodiscard]] std::vector<TimingStats> getAllTimingStats() const override;
    void resetTimingStats() override;

    // Snapshot & Export
    [[nodiscard]] PerformanceSnapshot takeSnapshot() const override;
    [[nodiscard]] std::string exportReportAsJson() const override;
    void exportReportToFile(const std::filesystem::path& path) const override;

    // Callback - uses NotificationHelper::registerCallback/unregisterCallback

protected:
    void initService() override;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

} // namespace ucf::service
