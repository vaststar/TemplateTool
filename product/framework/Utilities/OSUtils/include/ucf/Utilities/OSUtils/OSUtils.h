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
class Utilities_EXPORT OSUtils final
{
public:
    static OSType getOSType();
    static std::string getOSTypeName();
    static std::string getOSVersion();
};
}