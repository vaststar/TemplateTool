#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace ucf::service {

// Structure to hold information about a detected hang
struct HangInfo
{
    // When the hang was detected
    std::chrono::system_clock::time_point detectionTime;
    
    // How long the UI thread was unresponsive (in milliseconds)
    std::chrono::milliseconds hangDuration;
    
    // Application version when hang occurred
    std::string appVersion;
    
    // Product name
    std::string productName;
    
    // Stack trace of the hung thread (if available)
    std::string stackTrace;
    
    // Path to the hang report file
    std::filesystem::path reportPath;
    
    // Whether the hang was recovered (true) or led to forced termination (false)
    bool recovered = true;
    
    // Additional context info
    std::string additionalInfo;
};

} // namespace ucf::service
