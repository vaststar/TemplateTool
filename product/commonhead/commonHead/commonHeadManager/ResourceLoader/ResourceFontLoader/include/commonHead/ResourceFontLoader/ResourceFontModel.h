#pragma once

#include <string>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::model{
enum class FontFamily{
    SegoeUI,
    Consolas,
    SegoeUIEmoji
};

enum class FontSize{
    ExtraSmall,     //12
    Small,          //14
    MediumSmall,    //16
    Medium,         //20
    Large,          //26
    ExtraLarge      //40
};

enum class FontWeight{
    Thin,
    ExtraLight,
    Light,
    Normal,
    Medium,
    DemiBold,
    Bold,
    ExtraBold,
    Black
};

struct COMMONHEAD_EXPORT Font
{
    std::string fontFamily;
    int fontSize{ 12 };
    int fontWeight{ 400 };
    bool isItalic{ false };
};
}