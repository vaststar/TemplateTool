#pragma once

#include <map>
#include <commonHead/ResourceColorLoader/model/Color.h>
#include <ColorPaletteModel.h>

namespace commonHead::model{
    
struct ColorSet
{
    ColorToken token;
    ColorPaletteType normal;
    ColorPaletteType hovered;
    ColorPaletteType pressed;
    ColorPaletteType disabled;
    ColorPaletteType focused;
    ColorPaletteType checked;
    ColorPaletteType selected;
};

struct ThemeColorSet
{
    ColorThemeType themeType;
    std::map<ColorToken, ColorSet> colorSets;
};
}