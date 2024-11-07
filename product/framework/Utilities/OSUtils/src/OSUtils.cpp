
#include <ucf/Utilities/OSUtils/OSUtils.h>

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
}