#pragma once

#include <QObject>
#include <UIDataStruct/UIDataStructExport.h>

namespace UILanguage{
Q_NAMESPACE_EXPORT(UIDataStruct_EXPORT)
void registerMetaObject();

enum class LanguageType {
    LanguageType_ENGLISH,
    LanguageType_CHINESE_SIMPLIFIED,
    LanguageType_CHINESE_TRADITIONAL,
    LanguageType_FRENCH,
    LanguageType_GERMAN,
    LanguageType_ITALIAN,
    LanguageType_SPANISH,
    LanguageType_PORTUGUESE,
    LanguageType_JAPANESE,
    LanguageType_KOREAN,
    LanguageType_RUSSIAN
};
Q_ENUM_NS(LanguageType)

}