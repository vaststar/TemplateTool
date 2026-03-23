#pragma once

#include <QObject>
#include <TranslatorManager/TranslatorManagerExport.h>

namespace commonHead::viewModels::model {
    enum class LanguageType;
}

namespace UILanguage{
Q_NAMESPACE_EXPORT(TranslatorManager_EXPORT)
void TranslatorManager_EXPORT registerMetaObject();

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

// 语言转换工具函数
TranslatorManager_EXPORT LanguageType convertFromViewModel(commonHead::viewModels::model::LanguageType language);
TranslatorManager_EXPORT commonHead::viewModels::model::LanguageType convertToViewModel(LanguageType language);

}
