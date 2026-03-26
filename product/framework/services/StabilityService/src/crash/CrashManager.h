#pragma once

#include <ucf/Services/StabilityService/CrashInfo.h>

#include <filesystem>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <memory>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

// Forward declaration
class IPlatformCrashHandler;

class CrashManager final
{
public:
    explicit CrashManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~CrashManager();
    
    CrashManager(const CrashManager&) = delete;
    CrashManager(CrashManager&&) = delete;
    CrashManager& operator=(const CrashManager&) = delete;
    CrashManager& operator=(CrashManager&&) = delete;

public:
    // Initialize (get path from ClientInfoService and install)
    void initialize();
    
    // Cleanup
    void cleanup();
    
    // Status query
    [[nodiscard]] bool isInstalled() const { return mInstalled; }
    [[nodiscard]] const std::filesystem::path& getCrashDirectory() const { return mCrashDirectory; }

    // Crash report
    [[nodiscard]] bool hasPendingCrashReport() const;
    [[nodiscard]] std::optional<CrashInfo> getLastCrashInfo() const;
    [[nodiscard]] std::vector<std::filesystem::path> getCrashReportFiles() const;
    void clearPendingCrashReport();
    void clearAllCrashReports();

    // Testing
    void forceCrashForTesting();
    
    // Crash callback (called by platform handler)
    void onCrash(int signalCode, const char* signalName);

private:
    void setCrashDirectory(const std::filesystem::path& dir);
    void install();
    void uninstall();
    
    void writeCrashLog(int signalCode, const char* signalName, const std::string& stackTrace);
    std::string formatTimestamp(std::chrono::system_clock::time_point tp) const;
    std::filesystem::path generateCrashLogPath() const;
    std::optional<CrashInfo> parseCrashLog(const std::filesystem::path& path) const;

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
    std::filesystem::path mCrashDirectory;
    bool mInstalled = false;
    
    // Application info
    std::string mAppVersion;
    std::string mProductName;
    
    // Platform handler (managed via interface, lifecycle managed by Manager)
    std::unique_ptr<IPlatformCrashHandler> mPlatformHandler;
};

} // namespace ucf::service
