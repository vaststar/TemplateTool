#pragma once

#include <map>
#include <commonHead/ResourceFontLoader/model/Font.h>
#include <FontToken.h>
#include <FontPaletteModel.h>

namespace commonHead::model{
    
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
}