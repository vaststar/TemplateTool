#pragma once

#include <ucf/Services/StabilityService/HangInfo.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class IPlatformHangHandler;

class HangManager final
{
public:
    explicit HangManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~HangManager();
    
    HangManager(const HangManager&) = delete;
    HangManager(HangManager&&) = delete;
    HangManager& operator=(const HangManager&) = delete;
    HangManager& operator=(HangManager&&) = delete;

public:
    // Initialize hang detection (starts watchdog thread)
    void initialize();
    
    // Cleanup (stops watchdog thread)
    void cleanup();
    
    // Called by UI layer to report heartbeat
    void reportHeartbeat();
    
    // Status query
    [[nodiscard]] bool isEnabled() const { return mEnabled; }
    [[nodiscard]] const std::filesystem::path& getHangDirectory() const { return mHangDirectory; }

    // Hang report
    [[nodiscard]] bool hasPendingHangReport() const;
    [[nodiscard]] std::optional<HangInfo> getLastHangInfo() const;
    [[nodiscard]] std::vector<std::filesystem::path> getHangReportFiles() const;
    void clearPendingHangReport();
    void clearAllHangReports();
    
    // Testing
    void forceHangForTesting();

    // Configuration
    void setHangThreshold(std::chrono::milliseconds threshold);
    [[nodiscard]] std::chrono::milliseconds getHangThreshold() const { return mHangThreshold; }

private:
    // Watchdog thread
    void startWatchdog();
    void stopWatchdog();
    void watchdogLoop();
    
    // Hang event handlers
    void onHangDetected(std::chrono::milliseconds hangDuration, const std::string& stackTrace);
    void onHangRecovered(std::chrono::milliseconds totalHangDuration);
    
    // Stack capture
    std::string captureMainThreadStack();
    
    // File operations
    void setHangDirectory(const std::filesystem::path& dir);
    void writeHangLog(const HangInfo& hangInfo);
    std::string formatTimestamp(std::chrono::system_clock::time_point tp) const;
    std::filesystem::path generateHangLogPath() const;
    std::optional<HangInfo> parseHangLog(const std::filesystem::path& path) const;

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
    std::filesystem::path mHangDirectory;
    bool mEnabled = false;
    
    // Configuration
    std::atomic<std::chrono::milliseconds> mHangThreshold{std::chrono::milliseconds{5000}};
    
    // Application info
    std::string mAppVersion;
    std::string mProductName;
    
    // Watchdog thread
    std::thread mWatchdogThread;
    std::atomic<bool> mWatchdogRunning{false};
    std::atomic<bool> mStopRequested{false};
    std::mutex mWatchdogMutex;
    std::condition_variable mWatchdogCondition;
    
    // Heartbeat tracking
    std::thread::id mMainThreadId;
    std::atomic<std::chrono::steady_clock::time_point> mLastHeartbeat;
    std::atomic<bool> mHangDetected{false};
    std::chrono::steady_clock::time_point mHangStartTime;
    
    // Platform handler for stack capture
    std::unique_ptr<IPlatformHangHandler> mPlatformHandler;
    
    // Thread safety for file operations
    mutable std::mutex mFileMutex;
};

} // namespace ucf::service
