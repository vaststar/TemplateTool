#include "HangManager.h"
#include "platform/IPlatformHangHandler.h"
#include "../StabilityServiceLogger.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

namespace ucf::service {

HangManager::HangManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
    , mMainThreadId(std::this_thread::get_id())
    , mLastHeartbeat(std::chrono::steady_clock::now())
{
    CRASHHANDLER_LOG_DEBUG("HangManager created");
}

HangManager::~HangManager()
{
    cleanup();
    CRASHHANDLER_LOG_DEBUG("HangManager destroyed");
}

void HangManager::initialize()
{
    CRASHHANDLER_LOG_INFO("HangManager::initialize() starting...");
    
    // Get configuration from ClientInfoService
    if (auto coreFramework = mCoreFramework.lock())
    {
        if (auto clientInfoService = coreFramework->getService<IClientInfoService>().lock())
        {
            // Get hang directory from ClientInfoService
            setHangDirectory(clientInfoService->getAppHangStoragePath());
            
            // Get application version
            mAppVersion = clientInfoService->getApplicationVersion().toString();
            
            // Get product info
            auto productInfo = clientInfoService->getProductInfo();
            mProductName = productInfo.productName;
            
            // Create platform handler
            mPlatformHandler = IPlatformHangHandler::create();
            
            // Start watchdog
            startWatchdog();
            mEnabled = true;
            
            CRASHHANDLER_LOG_INFO("HangManager initialized with threshold: " 
                << mHangThreshold.load().count() << "ms");
        }
        else
        {
            CRASHHANDLER_LOG_ERROR("Failed to get ClientInfoService");
        }
    }
    else
    {
        CRASHHANDLER_LOG_ERROR("Failed to get CoreFramework");
    }
}

void HangManager::cleanup()
{
    stopWatchdog();
    mPlatformHandler.reset();
    mEnabled = false;
    CRASHHANDLER_LOG_INFO("HangManager cleaned up");
}

void HangManager::setHangDirectory(const std::filesystem::path& dir)
{
    mHangDirectory = dir;
    
    // Ensure directory exists
    if (!ucf::utilities::FilePathUtils::EnsureDirectoryExists(mHangDirectory))
    {
        CRASHHANDLER_LOG_ERROR("Failed to create hang directory: " << mHangDirectory.string());
    }
    else
    {
        CRASHHANDLER_LOG_INFO("Hang directory set to: " << mHangDirectory.string());
    }
}

void HangManager::reportHeartbeat()
{
    auto now = std::chrono::steady_clock::now();
    mLastHeartbeat.store(now);
    
    // If we were in a hang state and now recovered
    if (mHangDetected.exchange(false))
    {
        auto hangDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - mHangStartTime);
        onHangRecovered(hangDuration);
    }
}

void HangManager::setHangThreshold(std::chrono::milliseconds threshold)
{
    mHangThreshold = threshold;
    CRASHHANDLER_LOG_DEBUG("Hang threshold updated to: " << threshold.count() << "ms");
}

// ============================================================================
// Watchdog Thread
// ============================================================================

void HangManager::startWatchdog()
{
    if (mWatchdogRunning)
    {
        return;
    }
    
    mStopRequested = false;
    mLastHeartbeat = std::chrono::steady_clock::now();
    mHangDetected = false;
    
    mWatchdogThread = std::thread(&HangManager::watchdogLoop, this);
    mWatchdogRunning = true;
    
    CRASHHANDLER_LOG_INFO("Watchdog started");
}

void HangManager::stopWatchdog()
{
    if (!mWatchdogRunning)
    {
        return;
    }
    
    mStopRequested = true;
    
    // Wake up the watchdog thread
    {
        std::lock_guard<std::mutex> lock(mWatchdogMutex);
        mWatchdogCondition.notify_all();
    }
    
    if (mWatchdogThread.joinable())
    {
        mWatchdogThread.join();
    }
    
    mWatchdogRunning = false;
    CRASHHANDLER_LOG_INFO("Watchdog stopped");
}

void HangManager::watchdogLoop()
{
    CRASHHANDLER_LOG_DEBUG("Watchdog thread started");
    
    // Check interval (half of threshold for responsiveness)
    auto checkInterval = mHangThreshold.load() / 2;
    if (checkInterval < std::chrono::milliseconds(100))
    {
        checkInterval = std::chrono::milliseconds(100);
    }
    
    while (!mStopRequested)
    {
        // Wait for check interval
        {
            std::unique_lock<std::mutex> lock(mWatchdogMutex);
            mWatchdogCondition.wait_for(lock, checkInterval, [this]()
            {
                return mStopRequested.load();
            });
        }
        
        if (mStopRequested)
        {
            break;
        }
        
        // Check heartbeat
        auto now = std::chrono::steady_clock::now();
        auto lastHeartbeat = mLastHeartbeat.load();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastHeartbeat);
        
        if (elapsed > mHangThreshold.load())
        {
            if (!mHangDetected)
            {
                // First detection of this hang
                mHangDetected = true;
                mHangStartTime = lastHeartbeat;
                
                CRASHHANDLER_LOG_WARN("Hang detected! No heartbeat for " << elapsed.count() << "ms");
                
                // Capture stack trace and handle
                std::string stackTrace = captureMainThreadStack();
                onHangDetected(elapsed, stackTrace);
            }
            else
            {
                // Ongoing hang
                CRASHHANDLER_LOG_DEBUG("Hang ongoing, duration: " << elapsed.count() << "ms");
            }
        }
        
        // Update check interval if threshold changed
        auto newCheckInterval = mHangThreshold.load() / 2;
        if (newCheckInterval < std::chrono::milliseconds(100))
        {
            newCheckInterval = std::chrono::milliseconds(100);
        }
        checkInterval = newCheckInterval;
    }
    
    CRASHHANDLER_LOG_DEBUG("Watchdog thread exiting");
}

std::string HangManager::captureMainThreadStack()
{
    if (!mPlatformHandler)
    {
        return "[Platform hang handler not available]";
    }
    
    std::ostringstream oss;
    oss << "[Hang detected]\n";
    oss << "Threshold: " << mHangThreshold.load().count() << "ms\n";
    oss << "Main thread capture supported: " 
        << (mPlatformHandler->isMainThreadCaptureSupported() ? "yes" : "no") << "\n\n";
    
    oss << mPlatformHandler->captureMainThreadStack(mMainThreadId);
    
    return oss.str();
}

// ============================================================================
// Hang Report Management
// ============================================================================

bool HangManager::hasPendingHangReport() const
{
    std::lock_guard<std::mutex> lock(mFileMutex);
    
    if (!std::filesystem::exists(mHangDirectory))
    {
        return false;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(mHangDirectory))
    {
        if (entry.path().extension() == ".hang")
        {
            return true;
        }
    }
    return false;
}

std::optional<HangInfo> HangManager::getLastHangInfo() const
{
    std::lock_guard<std::mutex> lock(mFileMutex);
    
    auto files = getHangReportFiles();
    if (files.empty())
    {
        return std::nullopt;
    }
    
    // Sort by modification time, newest first
    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b)
    {
        return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
    });
    
    return parseHangLog(files.front());
}

std::vector<std::filesystem::path> HangManager::getHangReportFiles() const
{
    std::vector<std::filesystem::path> files;
    
    if (!std::filesystem::exists(mHangDirectory))
    {
        return files;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(mHangDirectory))
    {
        if (entry.path().extension() == ".hang")
        {
            files.push_back(entry.path());
        }
    }
    
    return files;
}

void HangManager::clearPendingHangReport()
{
    std::lock_guard<std::mutex> lock(mFileMutex);
    
    auto files = getHangReportFiles();
    if (!files.empty())
    {
        // Sort by modification time
        std::sort(files.begin(), files.end(), [](const auto& a, const auto& b)
        {
            return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
        });
        
        // Remove the newest one
        std::filesystem::remove(files.front());
        CRASHHANDLER_LOG_INFO("Cleared pending hang report: " << files.front().string());
    }
}

void HangManager::clearAllHangReports()
{
    std::lock_guard<std::mutex> lock(mFileMutex);
    
    auto files = getHangReportFiles();
    for (const auto& file : files)
    {
        std::filesystem::remove(file);
    }
    CRASHHANDLER_LOG_INFO("Cleared all hang reports, count: " << files.size());
}

void HangManager::forceHangForTesting()
{
    CRASHHANDLER_LOG_WARN("forceHangForTesting: Intentionally blocking main thread for hang detection test");
    
    // Sleep for longer than the hang threshold to trigger hang detection
    // Default threshold is 5 seconds, so sleep for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    CRASHHANDLER_LOG_INFO("forceHangForTesting: Main thread resumed after intentional hang");
}

void HangManager::onHangDetected(std::chrono::milliseconds hangDuration, const std::string& stackTrace)
{
    CRASHHANDLER_LOG_WARN("Hang detected! Duration: " << hangDuration.count() << "ms");
    
    HangInfo info;
    info.detectionTime = std::chrono::system_clock::now();
    info.hangDuration = hangDuration;
    info.appVersion = mAppVersion;
    info.productName = mProductName;
    info.stackTrace = stackTrace;
    info.recovered = false;  // Will be updated if recovered
    
    writeHangLog(info);
}

void HangManager::onHangRecovered(std::chrono::milliseconds totalHangDuration)
{
    CRASHHANDLER_LOG_INFO("Hang recovered after: " << totalHangDuration.count() << "ms");
    
    // Update the last hang report to mark as recovered
    // (In a more sophisticated implementation, we'd update the existing file)
}

void HangManager::writeHangLog(const HangInfo& hangInfo)
{
    std::lock_guard<std::mutex> lock(mFileMutex);
    
    auto logPath = generateHangLogPath();
    std::ofstream file(logPath);
    
    if (!file.is_open())
    {
        CRASHHANDLER_LOG_ERROR("Failed to open hang log file: " << logPath.string());
        return;
    }
    
    file << "=== Hang Report ===" << std::endl;
    file << "Time: " << formatTimestamp(hangInfo.detectionTime) << std::endl;
    file << "Duration: " << hangInfo.hangDuration.count() << "ms" << std::endl;
    file << "Product: " << hangInfo.productName << std::endl;
    file << "Version: " << hangInfo.appVersion << std::endl;
    file << "Recovered: " << (hangInfo.recovered ? "Yes" : "No") << std::endl;
    file << std::endl;
    file << "=== Stack Trace ===" << std::endl;
    file << hangInfo.stackTrace << std::endl;
    
    file.close();
    CRASHHANDLER_LOG_INFO("Hang log written to: " << logPath.string());
}

std::string HangManager::formatTimestamp(std::chrono::system_clock::time_point tp) const
{
    auto time_t_val = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_val{};
#if defined(_WIN32)
    localtime_s(&tm_val, &time_t_val);
#else
    localtime_r(&time_t_val, &tm_val);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::filesystem::path HangManager::generateHangLogPath() const
{
    auto now = std::chrono::system_clock::now();
    auto time_t_val = std::chrono::system_clock::to_time_t(now);
    std::tm tm_val{};
#if defined(_WIN32)
    localtime_s(&tm_val, &time_t_val);
#else
    localtime_r(&time_t_val, &tm_val);
#endif
    
    std::ostringstream oss;
    oss << "hang_" << std::put_time(&tm_val, "%Y%m%d_%H%M%S") << ".hang";
    
    return mHangDirectory / oss.str();
}

std::optional<HangInfo> HangManager::parseHangLog(const std::filesystem::path& path) const
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return std::nullopt;
    }
    
    HangInfo info;
    info.reportPath = path;
    
    std::string line;
    bool inStackTrace = false;
    std::ostringstream stackTrace;
    
    while (std::getline(file, line))
    {
        if (line.find("Duration:") == 0)
        {
            auto pos = line.find(':');
            if (pos != std::string::npos)
            {
                auto durationStr = line.substr(pos + 2);
                auto msPos = durationStr.find("ms");
                if (msPos != std::string::npos)
                {
                    durationStr = durationStr.substr(0, msPos);
                    info.hangDuration = std::chrono::milliseconds(std::stoll(durationStr));
                }
            }
        }
        else if (line.find("Product:") == 0)
        {
            auto pos = line.find(':');
            if (pos != std::string::npos)
            {
                info.productName = line.substr(pos + 2);
            }
        }
        else if (line.find("Version:") == 0)
        {
            auto pos = line.find(':');
            if (pos != std::string::npos)
            {
                info.appVersion = line.substr(pos + 2);
            }
        }
        else if (line.find("Recovered:") == 0)
        {
            info.recovered = (line.find("Yes") != std::string::npos);
        }
        else if (line.find("=== Stack Trace ===") != std::string::npos)
        {
            inStackTrace = true;
        }
        else if (inStackTrace)
        {
            stackTrace << line << "\n";
        }
    }
    
    info.stackTrace = stackTrace.str();
    return info;
}

} // namespace ucf::service
