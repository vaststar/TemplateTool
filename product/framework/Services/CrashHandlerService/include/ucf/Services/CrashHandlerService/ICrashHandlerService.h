#pragma once

#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/CrashHandlerService/CrashInfo.h>

#include <memory>
#include <functional>
#include <filesystem>
#include <optional>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
}

namespace ucf::service {

class SERVICE_EXPORT ICrashHandlerService : public IService
{
public:
    ~ICrashHandlerService() override = default;

    // ==========================================
    // Status Query
    // ==========================================
    
    /// Check if crash handler is installed
    [[nodiscard]] virtual bool isInstalled() const = 0;

    // ==========================================
    // Crash Report Query
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
    // Testing
    // ==========================================
    
    /// Force a crash for testing purposes only
    virtual void forceCrashForTesting() = 0;

    // ==========================================
    // Factory Method
    // ==========================================
    
    [[nodiscard]] static std::shared_ptr<ICrashHandlerService> 
        createInstance(ucf::framework::ICoreFrameworkPtr coreFramework);
};

using ICrashHandlerServicePtr = std::shared_ptr<ICrashHandlerService>;
using ICrashHandlerServiceWPtr = std::weak_ptr<ICrashHandlerService>;

} // namespace ucf::service
