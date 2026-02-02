#pragma once

#include <string>
#include <filesystem>
#include <chrono>

namespace ucf::service {

struct CrashInfo
{
    // Signal/Exception info
    std::string signalName;         // "SIGSEGV", "SIGABRT", "EXCEPTION_ACCESS_VIOLATION"
    int signalCode = 0;             // Signal/Exception code
    
    // Timestamp
    std::chrono::system_clock::time_point timestamp;
    std::string timestampStr;       // "2026-02-02 14:30:45"
    
    // Stack trace
    std::string stackTrace;         // Human-readable stack trace
    
    // File paths
    std::filesystem::path crashLogPath;     // Crash log path
    std::filesystem::path minidumpPath;     // Minidump path (Windows only)
    
    // Application info
    std::string appVersion;
    std::string osVersion;
};

} // namespace ucf::service
