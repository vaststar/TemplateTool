#pragma once

#include <string>
#include <format>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service::model{

struct SERVICE_EXPORT Version
{
    std::string majorVersion;
    std::string minorVersion;
    std::string patchVersion;
    std::string buildVersion;
    std::string toString() const{
        return std::format("{}.{}.{}.{}", majorVersion, minorVersion, patchVersion, buildVersion);
    }
};

struct SERVICE_EXPORT ProductInfo
{
    std::string companyName;
    std::string copyright;
    std::string productName;
    std::string productDescription;
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
