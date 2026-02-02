#include "CrashHandlerManager.h"
#include "CrashHandlerServiceLogger.h"
#include "platform/IPlatformCrashHandler.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace ucf::service {

CrashHandlerManager::CrashHandlerManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
{
    CRASHHANDLER_LOG_DEBUG("CrashHandlerManager created");
}

CrashHandlerManager::~CrashHandlerManager()
{
    cleanup();
    CRASHHANDLER_LOG_DEBUG("CrashHandlerManager destroyed");
}

void CrashHandlerManager::initialize()
{
    CRASHHANDLER_LOG_INFO("CrashHandlerManager::initialize() starting...");
    
    // Get configuration from ClientInfoService
    if (auto coreFramework = mCoreFramework.lock())
    {
        CRASHHANDLER_LOG_DEBUG("CoreFramework obtained");
        if (auto clientInfoService = coreFramework->getService<IClientInfoService>().lock())
        {
            CRASHHANDLER_LOG_DEBUG("ClientInfoService obtained");
            // Get crash directory
            setCrashDirectory(clientInfoService->getAppCrashStoragePath());
            
            // Get application version
            mAppVersion = clientInfoService->getApplicationVersion().toString();
            
            // Get product info
            auto productInfo = clientInfoService->getProductInfo();
            mProductName = productInfo.productName;
            
            install();
            CRASHHANDLER_LOG_INFO("CrashHandler initialized with version: " << mAppVersion);
        }
        else
        {
            CRASHHANDLER_LOG_ERROR("Failed to get ClientInfoService, crash handler not installed");
        }
    }
    else
    {
        CRASHHANDLER_LOG_ERROR("CoreFramework is null, crash handler not installed");
    }
}

void CrashHandlerManager::cleanup()
{
    uninstall();
}

void CrashHandlerManager::setCrashDirectory(const std::filesystem::path& dir)
{
    mCrashDirectory = dir;
    
    // Ensure directory exists
    if (!ucf::utilities::FilePathUtils::EnsureDirectoryExists(mCrashDirectory))
    {
        CRASHHANDLER_LOG_ERROR("Failed to create crash directory: " << dir);
    }
}

void CrashHandlerManager::install()
{
    if (mInstalled)
    {
        CRASHHANDLER_LOG_WARN("CrashHandler already installed");
        return;
    }
    
    if (mCrashDirectory.empty())
    {
        CRASHHANDLER_LOG_ERROR("Crash directory not set");
        return;
    }
    
    // Create platform handler via factory method (auto-installed on construction)
    mPlatformHandler = IPlatformCrashHandler::create({
        .callback = [this](int signalCode, const char* signalName) {
            this->onCrash(signalCode, signalName);
        },
        .crashDir = mCrashDirectory
    });
    
    if (!mPlatformHandler)
    {
        CRASHHANDLER_LOG_ERROR("Failed to create platform crash handler");
        return;
    }
    
    mInstalled = true;
    CRASHHANDLER_LOG_INFO("CrashHandler installed, crash directory: " << mCrashDirectory);
}

void CrashHandlerManager::uninstall()
{
    if (!mInstalled)
    {
        return;
    }
    
    // Destroy platform handler (auto-uninstalled on destruction)
    mPlatformHandler.reset();
    
    mInstalled = false;
    CRASHHANDLER_LOG_INFO("CrashHandler uninstalled");
}

bool CrashHandlerManager::hasPendingCrashReport() const
{
    auto files = getCrashReportFiles();
    return !files.empty();
}

std::optional<CrashInfo> CrashHandlerManager::getLastCrashInfo() const
{
    auto files = getCrashReportFiles();
    if (files.empty())
    {
        return std::nullopt;
    }
    
    // Find the newest file
    auto newest = std::max_element(files.begin(), files.end(), [](const auto& a, const auto& b){
        return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
    });
    
    return parseCrashLog(*newest);
}

std::vector<std::filesystem::path> CrashHandlerManager::getCrashReportFiles() const
{
    std::vector<std::filesystem::path> result;
    
    if (mCrashDirectory.empty() || !std::filesystem::exists(mCrashDirectory))
    {
        return result;
    }
    
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(mCrashDirectory, ec))
    {
        if (entry.path().extension() == ".crash")
        {
            result.push_back(entry.path());
        }
    }
    
    return result;
}

void CrashHandlerManager::clearPendingCrashReport()
{
    auto files = getCrashReportFiles();
    if (files.empty())
    {
        return;
    }
    
    // Only delete the newest one
    auto newest = std::max_element(files.begin(), files.end(), [](const auto& a, const auto& b) {
        return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
    });
    
    std::error_code ec;
    std::filesystem::remove(*newest, ec);
}

void CrashHandlerManager::clearAllCrashReports()
{
    auto files = getCrashReportFiles();
    std::error_code ec;
    for (const auto& file : files)
    {
        std::filesystem::remove(file, ec);
    }
}

void CrashHandlerManager::forceCrashForTesting()
{
    CRASHHANDLER_LOG_WARN("Forcing crash for testing...");
    
    // Use null pointer dereference to trigger access violation
    // This will be caught by SetUnhandledExceptionFilter on Windows
    // and by signal handlers on POSIX systems
    volatile int* p = nullptr;
    *p = 42;  // NOLINT - intentional crash for testing
}

void CrashHandlerManager::onCrash(int signalCode, const char* signalName)
{
    // WARNING: This function is called from signal handler, only use async-signal-safe functions
    
    std::string stackTrace;
    if (mPlatformHandler)
    {
        stackTrace = mPlatformHandler->captureStackTrace(3); // Skip signal handling related frames
    }
    
    writeCrashLog(signalCode, signalName, stackTrace);
}

void CrashHandlerManager::writeCrashLog(int signalCode, const char* signalName, 
                                         const std::string& stackTrace)
{
    auto logPath = generateCrashLogPath();
    
    // Use C API for file writing (closer to async-signal-safe)
    // Use path.string() for cross-platform compatibility (path::c_str() returns wchar_t* on Windows)
    FILE* fp = fopen(logPath.string().c_str(), "w");
    if (!fp)
    {
        return;
    }
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = formatTimestamp(now);
    
    fprintf(fp, "================== CRASH REPORT ==================\n");
    fprintf(fp, "Time: %s\n", timestamp.c_str());
    fprintf(fp, "Signal: %s (code: %d)\n", signalName, signalCode);
    fprintf(fp, "App Version: %s\n", mAppVersion.c_str());
    fprintf(fp, "Product: %s\n", mProductName.c_str());
    fprintf(fp, "\n");
    
    fprintf(fp, "=================== STACK TRACE ==================\n");
    fprintf(fp, "%s", stackTrace.c_str());
    fprintf(fp, "\n");
    fprintf(fp, "==================================================\n");
    
    fclose(fp);
}

std::string CrashHandlerManager::formatTimestamp(std::chrono::system_clock::time_point tp) const
{
    auto time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return buffer;
}

std::filesystem::path CrashHandlerManager::generateCrashLogPath() const
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "crash_%Y%m%d_%H%M%S.crash", &tm);
    
    return mCrashDirectory / buffer;
}

std::optional<CrashInfo> CrashHandlerManager::parseCrashLog(const std::filesystem::path& path) const
{
    std::ifstream file(path);
    if (!file)
    {
        return std::nullopt;
    }
    
    CrashInfo info;
    info.crashLogPath = path;
    
    std::string line;
    bool inStackTrace = false;
    std::ostringstream stackTrace;
    
    while (std::getline(file, line))
    {
        if (line.find("Time:") == 0)
        {
            info.timestampStr = line.substr(6);
        }
        else if (line.find("Signal:") == 0)
        {
            // Parse "Signal: SIGSEGV (code: 11)"
            auto pos = line.find("(code:");
            if (pos != std::string::npos)
            {
                info.signalName = line.substr(8, pos - 9);
                info.signalCode = std::stoi(line.substr(pos + 7));
            }
        }
        else if (line.find("STACK TRACE") != std::string::npos)
        {
            inStackTrace = true;
        }
        else if (inStackTrace && line.find("====") == std::string::npos)
        {
            stackTrace << line << "\n";
        }
    }
    
    info.stackTrace = stackTrace.str();
    return info;
}

} // namespace ucf::service
