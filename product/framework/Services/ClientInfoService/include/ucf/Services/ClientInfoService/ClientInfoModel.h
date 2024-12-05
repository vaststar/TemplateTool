#pragma once

#include <string>
#include <format>

namespace ucf::service::model{

struct Version
{
    std::string majorVersion;
    std::string minorVersion;
    std::string patchVersion;
    std::string toString() const{
        return std::format("{}.{}.{}", majorVersion, minorVersion, patchVersion);
    }
};

enum class OSType
{
    Windows32,
    Windows64,
    OSX,
    Linux
};

enum class LanguageType {
    ENGLISH,
    CHINESE_SIMPLIFIED,
    CHINESE_TRADITIONAL,
    FRENCH,
    GERMAN,
    ITALIAN,
    SPANISH,
    PORTUGUESE,
    JAPANESE,
    KOREAN,
    RUSSIAN
};

}