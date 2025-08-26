#pragma once

#include <UIDataStruct/UIDataStructExport.h>

namespace commonHead::model{
enum class ColorItem;
enum class ColorState;
enum class FontFamily;
enum class FontSize;
enum class FontWeight;
}

namespace UIElementData{
enum class UIColorEnum;
enum class UIColorState;
enum class UIFontFamily;
enum class UIFontSize;
enum class UIFontWeight;
}

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

    static commonHead::model::FontFamily convertUIFontFamilyToVMFontFamily(UIElementData::UIFontFamily uiFontFamily);
    static commonHead::model::FontSize convertUIFontSizeToVMFontSize(UIElementData::UIFontSize uiFontSize);
    static commonHead::model::FontWeight convertUIFontWeightToVMFontWeight(UIElementData::UIFontWeight uiFontWeight);

    static UILanguage::LanguageType convertViewModelLanguageToUILanguage(commonHead::viewModels::model::LanguageType language);
    static commonHead::viewModels::model::LanguageType convertUILanguageToViewModelLanguage(UILanguage::LanguageType language);
};