#pragma once

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::model{
struct COMMONHEAD_EXPORT Color{
    int r{ 0 };
    int g{ 0 };
    int b{ 0 };
    float a{ 1.0 };
};

enum class ColorState{
    Normal,
    Hovered,
    Pressed,
    Disabled,
    Focused,
    Checked
}; 

enum class ColorThemeType{
    White,
    Dark
};
}
