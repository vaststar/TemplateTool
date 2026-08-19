#pragma once

#include <map>

#include <commonhead/ResourceColorLoader/ColorPaletteModel.h>
#include <commonhead/ResourceColorLoader/Color.h>

namespace commonHead::model {

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

} // namespace commonHead::model
