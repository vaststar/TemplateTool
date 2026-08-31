#pragma once

#include <ucf/utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/services/ServiceDeclaration/IService.h>
#include <ucf/services/PerformanceService/PerformanceInfo.h>
#include <ucf/services/PerformanceService/IPerformanceServiceCallback.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
}

namespace ucf::service {

class IPerformanceService : public IService,
                                           public virtual ucf::utilities::INotificationHelper<IPerformanceServiceCallback>
{
public:
    ~IPerformanceService() override = default;

    // ==========================================
    // Memory Monitoring
    // ==========================================

    /// Get the currently available process and system memory metrics.
    /// Unsupported or failed metrics are represented by nullopt fields.
    [[nodiscard]] virtual MemoryInfo getCurrentMemoryUsage() const = 0;

    /// Set the process resident-memory warning threshold in bytes.
    /// A value of 0 disables the warning.
    virtual void setProcessResidentMemoryWarningThreshold(uint64_t bytes) = 0;

    /// Get the process resident-memory warning threshold in bytes.
    [[nodiscard]] virtual uint64_t getProcessResidentMemoryWarningThreshold() const = 0;

    // ==========================================
    // CPU Monitoring
    // ==========================================

    /// Get current process CPU usage.
    /// 100% means one logical CPU core is fully utilized, so a multi-threaded
    /// process may exceed 100%. Returns nullopt until a valid sample exists.
    [[nodiscard]] virtual std::optional<double> getProcessCpuUsagePercent() const = 0;

    /// Get current system-wide CPU usage normalized to 0%-100%.
    /// Returns nullopt until a valid sample exists.
    [[nodiscard]] virtual std::optional<double> getSystemCpuUsagePercent() const = 0;

    /// Set the process CPU warning threshold in percent. The threshold follows
    /// the one-logical-core-equals-100% convention. 0 or negative = disabled.
    virtual void setCpuWarningThreshold(double percent) = 0;

    /// Get the current process CPU warning threshold in percent.
    [[nodiscard]] virtual double getCpuWarningThreshold() const = 0;

    // ==========================================
    // Timing
    // ==========================================

    /// Begin timing an operation
    /// @param operationName Name of the operation (e.g., "OpenDocument", "Search")
    /// @return Token to be passed to endTiming
    [[nodiscard]] virtual TimingToken beginTiming(const std::string& operationName) = 0;

    /// End timing and record the duration
    /// @param token Token returned by beginTiming
    virtual void endTiming(const TimingToken& token) = 0;

    /// Get timing statistics for a specific operation.
    /// Returns nullopt when the operation has no completed samples.
    [[nodiscard]] virtual std::optional<TimingStats> getTimingStats(
        const std::string& operationName) const = 0;

    /// Get all timing statistics
    [[nodiscard]] virtual std::vector<TimingStats> getAllTimingStats() const = 0;

    /// Reset all timing statistics
    virtual void resetTimingStats() = 0;

    // ==========================================
    // Snapshot & Export
    // ==========================================

    /// Take a complete performance snapshot
    [[nodiscard]] virtual PerformanceSnapshot takeSnapshot() const = 0;

    /// Export performance report as JSON string
    [[nodiscard]] virtual std::string exportReportAsJson() const = 0;

    /// Export performance report to file
    virtual void exportReportToFile(const std::filesystem::path& path) const = 0;

    // Callback - inherited from INotificationHelper
    // void registerCallback(std::weak_ptr<IPerformanceServiceCallback>)
    // void unregisterCallback(std::weak_ptr<IPerformanceServiceCallback>)
};

using IPerformanceServicePtr = std::shared_ptr<IPerformanceService>;
using IPerformanceServiceWPtr = std::weak_ptr<IPerformanceService>;

/// RAII helper for timing
class ScopedTiming
{
public:
    ScopedTiming(IPerformanceServiceWPtr service, const std::string& operationName)
        : mService(std::move(service))
    {
        if (auto lockedService = mService.lock())
        {
            mToken = lockedService->beginTiming(operationName);
        }
    }

    ~ScopedTiming()
    {
        if (auto lockedService = mService.lock())
        {
            if (mToken.isValid())
            {
                lockedService->endTiming(mToken);
            }
        }
    }

    ScopedTiming(const ScopedTiming&) = delete;
    ScopedTiming& operator=(const ScopedTiming&) = delete;

private:
    IPerformanceServiceWPtr mService;
    TimingToken mToken;
};

} // namespace ucf::service
