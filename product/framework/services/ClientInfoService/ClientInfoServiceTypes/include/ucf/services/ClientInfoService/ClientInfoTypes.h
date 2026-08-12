#pragma once

#include <string>
#include <format>

#include <ucf/services/ClientInfoService/ClientInfoServiceTypesExport.h>

namespace ucf::service::model{

struct CLIENT_INFO_SERVICE_TYPES_API Version
{
    std::string majorVersion;
    std::string minorVersion;
    std::string patchVersion;
    std::string buildVersion;
    std::string toString() const{
        return std::format("{}.{}.{}.{}", majorVersion, minorVersion, patchVersion, buildVersion);
    }
};

struct CLIENT_INFO_SERVICE_TYPES_API ProductInfo
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
