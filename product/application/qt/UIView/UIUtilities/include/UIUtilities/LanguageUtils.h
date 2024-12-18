#pragma once

#include <UIUtilities/UIUtilitiesExport.h>

namespace commonHead::viewModels::model{
    enum class LanguageType;
}
namespace UIManager{
enum class LanguageType;
}

class UIUtilities_EXPORT LanguageUtils final
{
public:
    static UIManager::LanguageType convertViewModelLanguageToUILanguage(commonHead::viewModels::model::LanguageType language);
    static commonHead::viewModels::model::LanguageType convertUILanguageToViewModelLanguage(UIManager::LanguageType language);
};
