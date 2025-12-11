#pragma once

#include <string>
#include <format>

namespace ucf::service::model{

struct Version
{
    std::string majorVersion;
    std::string minorVersion;
    std::string patchVersion;
    std::string buildVersion;
    std::string toString() const{
        return std::format("{}.{}.{}.{}", majorVersion, minorVersion, patchVersion, buildVersion);
    }
};

struct ProductInfo
{
    std::string companyName;
    std::string copyright;
    std::string productName;
    std::string productDescription;
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

enum class ThemeType {
    SystemDefault,
    Dark,
    Light
};
}