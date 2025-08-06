#pragma once

#include <string>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities{
enum class OSType{
    WINDOWS,
    MACOS,
    IOS,
    APPLE_VISION,
    ANDROID,
    LINUX,
    UNIX,
    UNKNOWN
};

struct MemoryInfo {
    uint64_t totalMemoryBytes;
    uint64_t availableMemoryBytes;
};

class Utilities_EXPORT OSUtils final
{
public:
    static OSType getOSType();
    static std::string getOSTypeName();
    static std::string getOSVersion();
    static unsigned int getCPUCoreCount();
    static std::string getCPUInfo();
    static std::string getCompilerInfo();
    static MemoryInfo getMemoryInfo();
    static std::string getSystemLanguage();
    static std::string getGPUInfo();
};
}