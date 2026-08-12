#pragma once

#include <commonHead/ResourceColorLoader/ResourceColorLoaderTypesExport.h>

namespace commonHead::model {

struct RESOURCE_COLOR_LOADER_TYPES_API Color
{
    int r{ 0 };
    int g{ 0 };
    int b{ 0 };
    float a{ 1.0 };
};

enum class ColorState
{
    Normal,
    Hovered,
    Pressed,
    Disabled,
    Focused,
    Checked,
    Selected
};

enum class ColorThemeType
{
    Light,
    Dark
};

} // namespace commonHead::model
