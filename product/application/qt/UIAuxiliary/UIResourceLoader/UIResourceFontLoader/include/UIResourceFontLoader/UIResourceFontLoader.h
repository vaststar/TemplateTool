#pragma once
#include <UIResourceFontLoader/UIResourceFontLoaderExport.h>

namespace commonHead::model{
    enum class FontFamily;
    enum class FontSize;
    enum class FontWeight;
}

namespace UIColorState{
    enum class ColorState;
}

namespace UIFontToken{
    enum class UIFontFamily;
    enum class UIFontSize;
    enum class UIFontWeight;
}
namespace UIResource{
class UIResourceFontLoader_EXPORT UIResourceFontLoader final
{
public:
    static void registerUIResourceFontLoader();
    static commonHead::model::FontFamily convertUIFontFamilyToVMFontFamily(UIFontToken::UIFontFamily uiFontFamily);
    static commonHead::model::FontSize convertUIFontSizeToVMFontSize(UIFontToken::UIFontSize uiFontSize);
    static commonHead::model::FontWeight convertUIFontWeightToVMFontWeight(UIFontToken::UIFontWeight uiFontWeight);
};
}