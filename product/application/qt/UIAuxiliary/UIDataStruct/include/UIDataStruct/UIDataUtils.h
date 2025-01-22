#pragma once

#include <UIDataStruct/UIDataStructExport.h>

namespace commonHead::model{
enum class ColorItem;
enum class ColorItemState;
enum class FontFamily;
enum class FontSize;
enum class FontWeight;
enum class LocalizedString;
enum class LocalizedStringWithParam;
}

namespace UIElementData{
enum class UIColorEnum;
enum class UIColorState;
enum class UIFontFamily;
enum class UIFontSize;
enum class UIFontWeight;
}

namespace UIStrings {
enum class LocalizedString;
enum class LocalizedStringWithParam;
}

class UIDataStruct_EXPORT UIDataUtils final
{
public:
    static void registerMetaObject();
    static commonHead::model::ColorItem convertUIColorEnumToVMColorItem(UIElementData::UIColorEnum  uiColorItem);
    static commonHead::model::ColorItemState convertUIColorStateToVMColorItemState(UIElementData::UIColorState uiColorState);

    static commonHead::model::FontFamily convertUIFontFamilyToVMFontFamily(UIElementData::UIFontFamily uiFontFamily);
    static commonHead::model::FontSize convertUIFontSizeToVMFontSize(UIElementData::UIFontSize uiFontSize);
    static commonHead::model::FontWeight convertUIFontWeightToVMFontWeight(UIElementData::UIFontWeight uiFontWeight);

    static commonHead::model::LocalizedString convertUILocalizedStringToVMLocalizedString(UIStrings::LocalizedString uiLocalizedString);
    static commonHead::model::LocalizedStringWithParam convertUILocalizedStringParamToVMLocalizedStringParam(UIStrings::LocalizedStringWithParam uiLocalizedStringWithParam);
};