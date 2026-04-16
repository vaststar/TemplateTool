
#include <ucf/Utilities/OSUtils/OSUtils.h>
#include <thread>

#if defined(_WIN32)
#include "OSUtils_Win.h"
#elif defined(__APPLE__)
#include "OSUtils_Mac.h"
#elif defined(__linux__)
#include "OSUtils_Linux.h"
#endif

namespace ucf::utilities{

OSType OSUtils::getOSType() {
    #if defined(_WIN32)
        return OSType::WINDOWS;
    #elif defined(__APPLE__)
        #include "TargetConditionals.h"
        #if defined(TARGET_OS_MAC) && TARGET_OS_MAC
            return OSType::MACOS;
        #elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
            return OSType::IOS;
        #elif defined(TARGET_OS_VISION) && TARGET_OS_VISION
            return OSType::APPLE_VISION;
        #else
            return OSType::UNKNOWN;
        #endif
    #elif defined(__ANDROID__)
        return OSType::ANDROID;
    #elif defined(__linux__)
        return OSType::LINUX;
    #elif defined(__unix__)
        return OSType::UNIX;
    #else
        return OSType::UNKNOWN;
    #endif
}

std::string OSUtils::getOSTypeName()
{
    switch (getOSType()) {
        case OSType::WINDOWS:
            return "Windows";
        case OSType::MACOS:
            return "macOS";
        case OSType::IOS:
            return "iOS";
        case OSType::ANDROID:
            return "Android";
        case OSType::LINUX:
            return "Linux";
        case OSType::UNIX:
            return "Unix";
        case OSType::APPLE_VISION:
            return "Apple Vision Pro";
        case OSType::UNKNOWN:
        default:
            return "Unknown OS";
    }
}

std::string OSUtils::getOSVersion()
{
#if defined(_WIN32)
    return OSUtils_Win::getOSVersion();
#elif defined(__APPLE__)
    return OSUtils_Mac::getOSVersion();
#elif defined(__linux__)
    return OSUtils_Linux::getOSVersion();
#endif
    return "Unknown Version";
}

unsigned int OSUtils::getCPUCoreCount()
{
    return std::thread::hardware_concurrency();
}

std::string OSUtils::getCPUInfo()
{
#if defined(_WIN32)
    return OSUtils_Win::getCPUInfo();
#elif defined(__APPLE__)
    return OSUtils_Mac::getCPUInfo();
#elif defined(__linux__)
    return OSUtils_Linux::getCPUInfo();
#endif
    return "Unknown CPU";
}


std::string OSUtils::getCompilerInfo()
{
#if defined(__clang__)
    return "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__);
#elif defined(__GNUC__)
    return "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#elif defined(_MSC_VER)
    return "MSVC " + std::to_string(_MSC_VER);
#else
    return "Unknown Compiler";
#endif
}

MemoryInfo OSUtils::getMemoryInfo()
{
    MemoryInfo memoryInfo = {0, 0};
#if defined(_WIN32)
    memoryInfo = OSUtils_Win::getMemoryInfo();
#elif defined(__APPLE__)
    memoryInfo = OSUtils_Mac::getMemoryInfo();
#elif defined(__linux__)
    memoryInfo = OSUtils_Linux::getMemoryInfo();
#endif
    return memoryInfo;
}

std::string OSUtils::getSystemLanguage()
{
    #if defined(_WIN32)
        return OSUtils_Win::getSystemLanguage();
    #elif defined(__APPLE__)
        return OSUtils_Mac::getSystemLanguage();
    #elif defined(__linux__)
        return OSUtils_Linux::getSystemLanguage();
    #endif
        return "Unknown Language";
}

std::string OSUtils::getGPUInfo()
{
    #if defined(_WIN32)
        return OSUtils_Win::getGPUInfo();
    #elif defined(__APPLE__)
        return OSUtils_Mac::getGPUInfo();
    #elif defined(__linux__)
        return OSUtils_Linux::getGPUInfo();
    #endif
        return "Unknown GPU";
}
}