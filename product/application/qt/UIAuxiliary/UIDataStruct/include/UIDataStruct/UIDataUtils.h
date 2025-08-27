#pragma once

#include <UIDataStruct/UIDataStructExport.h>

namespace UILanguage{
enum class LanguageType;
}

namespace commonHead::viewModels::model {
    enum class LanguageType;
}

class UIDataStruct_EXPORT UIDataUtils final
{
public:
    static void registerMetaObject();

    static UILanguage::LanguageType convertViewModelLanguageToUILanguage(commonHead::viewModels::model::LanguageType language);
    static commonHead::viewModels::model::LanguageType convertUILanguageToViewModelLanguage(UILanguage::LanguageType language);
};