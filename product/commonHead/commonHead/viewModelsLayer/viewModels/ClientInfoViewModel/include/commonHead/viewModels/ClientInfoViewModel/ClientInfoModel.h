#pragma once
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

#include <string>
#include <vector>

namespace commonHead::viewModels::model{
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