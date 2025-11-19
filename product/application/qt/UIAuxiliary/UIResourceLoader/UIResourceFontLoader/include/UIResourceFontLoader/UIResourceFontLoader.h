#pragma once
#include <UIResourceFontLoader/UIResourceFontLoaderExport.h>

namespace commonHead::model{
    enum class FontToken;
}

namespace UIFontToken{
    enum class FontToken;
}
namespace UIResource{
class UIResourceFontLoader_EXPORT UIResourceFontLoader final
{
public:
    static void registerUIResourceFontLoader();
    static commonHead::model::FontToken convertUIFontTokenToVMFontToken(UIFontToken::FontToken  uiFontItem);
};
}