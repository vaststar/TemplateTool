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
    UIResourceFontLoader() = delete;
    UIResourceFontLoader(const UIResourceFontLoader&) = delete;
    UIResourceFontLoader(UIResourceFontLoader&&) = delete;
    UIResourceFontLoader& operator=(const UIResourceFontLoader&) = delete;
    UIResourceFontLoader& operator=(UIResourceFontLoader&&) = delete;
    ~UIResourceFontLoader() = delete;
public:
    static void registerUIResourceFontLoader();
    static commonHead::model::FontToken convertUIFontTokenToVMFontToken(UIFontToken::FontToken  uiFontItem);
};
}