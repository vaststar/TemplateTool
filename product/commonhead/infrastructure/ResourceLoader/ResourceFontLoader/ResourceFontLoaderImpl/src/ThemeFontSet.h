#pragma once

#include <map>

#include <commonhead/ResourceFontLoader/FontPaletteModel.h>
#include <commonhead/ResourceFontLoader/FontToken.h>
#include <commonhead/ResourceFontLoader/Font.h>

namespace commonHead::model {

struct FontSet
{
    FontToken token;
    FontType fontType;
    FontWeight fontWeight;
    FontSize fontSize;
    bool isItalic{ false };
};

struct ThemeFontSet
{
    FontThemeType themeType;
    std::map<FontToken, FontSet> fontSets;
};

} // namespace commonHead::model
