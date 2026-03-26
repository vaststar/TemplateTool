#pragma once

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/PerformanceService/PerformanceInfo.h>
#include <ucf/Services/PerformanceService/IPerformanceServiceCallback.h>

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
}

namespace ucf::service {

class SERVICE_EXPORT IPerformanceService : public IService,
                                           public virtual ucf::utilities::INotificationHelper<IPerformanceServiceCallback>
{
public:
    ~IPerformanceService() override = default;

    // ==========================================
    // Memory Monitoring
    // ==========================================
    
    /// Get current memory usage
    [[nodiscard]] virtual MemoryInfo getCurrentMemoryUsage() const = 0;
    
    /// Set memory warning threshold (callback triggered when exceeded)
    virtual void setMemoryWarningThreshold(uint64_t bytes) = 0;
    
    /// Get current memory warning threshold
    [[nodiscard]] virtual uint64_t getMemoryWarningThreshold() const = 0;

    // ==========================================
    // CPU Monitoring
    // ==========================================
    
    /// Get current process CPU usage (0.0 - 100.0+)
    [[nodiscard]] virtual double getCPUUsage() const = 0;

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
    
    /// Get timing statistics for a specific operation
    [[nodiscard]] virtual TimingStats getTimingStats(const std::string& operationName) const = 0;
    
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

    // ==========================================
    // Factory Method
    // ==========================================
    
    [[nodiscard]] static std::shared_ptr<IPerformanceService> 
        createInstance(ucf::framework::ICoreFrameworkPtr coreFramework);
};

using IPerformanceServicePtr = std::shared_ptr<IPerformanceService>;
using IPerformanceServiceWPtr = std::weak_ptr<IPerformanceService>;

/// RAII helper for timing
class SERVICE_EXPORT ScopedTiming
{
public:
    ScopedTiming(IPerformanceServiceWPtr service, const std::string& operationName)
        : mService(service)
        , mToken(service.lock() ? service.lock()->beginTiming(operationName) : TimingToken{})
    {}
    
    ~ScopedTiming() {
        if (auto service = mService.lock()) {
            if (mToken.isValid()) {
                service->endTiming(mToken);
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
