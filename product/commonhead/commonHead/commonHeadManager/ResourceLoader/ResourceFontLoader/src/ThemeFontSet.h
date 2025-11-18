#pragma once

#include <map>
#include <commonHead/ResourceFontLoader/model/Font.h>
#include <FontPaletteModel.h>

namespace commonHead::model{
    
struct FontSet
{
    FontToken token;
    
};

struct ThemeFontSet
{
    FontThemeType themeType;
    std::map<FontToken, FontSet> fontSets;
};
}