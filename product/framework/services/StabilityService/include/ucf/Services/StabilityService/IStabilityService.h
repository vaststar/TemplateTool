#pragma once

#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/StabilityService/CrashInfo.h>
#include <ucf/Services/StabilityService/HangInfo.h>

#include <memory>
#include <functional>
#include <filesystem>
#include <optional>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
}

namespace ucf::service {

class SERVICE_EXPORT IStabilityService : public IService
{
public:
    ~IStabilityService() override = default;

    // ==========================================
    // Status Query
    // ==========================================
    
    /// Check if crash handler is installed
    [[nodiscard]] virtual bool isCrashHandlerInstalled() const = 0;

    // ==========================================
    // Crash Report
    // ==========================================
    
    /// Check if there is a pending crash report from last session
    [[nodiscard]] virtual bool hasPendingCrashReport() const = 0;
    
    /// Get last crash info
    [[nodiscard]] virtual std::optional<CrashInfo> getLastCrashInfo() const = 0;
    
    /// Get all crash report files
    [[nodiscard]] virtual std::vector<std::filesystem::path> getCrashReportFiles() const = 0;
    
    /// Clear the latest pending crash report
    virtual void clearPendingCrashReport() = 0;
    
    /// Clear all crash reports
    virtual void clearAllCrashReports() = 0;

    // ==========================================
    // Hang Detection
    // ==========================================
    
    /// Report heartbeat from UI layer (call this periodically from main thread)
    virtual void reportHeartbeat() = 0;
    
    /// Check if hang detection is enabled
    [[nodiscard]] virtual bool isHangDetectionEnabled() const = 0;
    
    /// Check if there is a pending hang report
    [[nodiscard]] virtual bool hasPendingHangReport() const = 0;
    
    /// Get last hang info
    [[nodiscard]] virtual std::optional<HangInfo> getLastHangInfo() const = 0;
    
    /// Get all hang report files
    [[nodiscard]] virtual std::vector<std::filesystem::path> getHangReportFiles() const = 0;
    
    /// Clear the latest pending hang report
    virtual void clearPendingHangReport() = 0;
    
    /// Clear all hang reports
    virtual void clearAllHangReports() = 0;

    // ==========================================
    // Testing
    // ==========================================
    
    /// Force a crash for testing purposes only
    virtual void forceCrashForTesting() = 0;
    
    /// Force a hang for testing purposes only (blocks main thread)
    virtual void forceHangForTesting() = 0;

    // ==========================================
    // Factory Method
    // ==========================================
    
    [[nodiscard]] static std::shared_ptr<IStabilityService> 
        createInstance(ucf::framework::ICoreFrameworkPtr coreFramework);
};

using IStabilityServicePtr = std::shared_ptr<IStabilityService>;
using IStabilityServiceWPtr = std::weak_ptr<IStabilityService>;

} // namespace ucf::service
