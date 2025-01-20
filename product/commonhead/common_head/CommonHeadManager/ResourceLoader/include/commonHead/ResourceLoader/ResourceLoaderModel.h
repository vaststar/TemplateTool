#pragma once

#include <string>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::model{
enum class ColorItem{
    ColorItem_Button_Primary_Text,
    ColorItem_Button_Primary_Background,
    ColorItem_Button_Primary_Border
};
enum class ColorItemState{
    ColorItemState_Normal,
    ColorItemState_Hovered,
    ColorItemState_Pressed,
    ColorItemState_Disabled,
    ColorItemState_Focused,
    ColorItemState_Checked
};

enum class FontFamily{
    FontFamily_SegoeUI,
    FontFamily_Consolas,
    FontFamily_SegoeUIEmoji
};

enum class FontSize{
    FontSize_ExtraSmall,     //12
    FontSize_Small,          //14
    FontSize_MediumSmall,    //16
    FontSize_Medium,         //20
    FontSize_Large,          //26
    FontSize_ExtraLarge      //40
};

enum class FontWeight{
    FontWeight_Thin,
    FontWeight_ExtraLight,
    FontWeight_Light,
    FontWeight_Normal,
    FontWeight_Medium,
    FontWeight_DemiBold,
    FontWeight_Bold,
    FontWeight_ExtraBold,
    FontWeight_Black
};

struct COMMONHEAD_EXPORT Color{
    int r{ 0 };
    int g{ 0 };
    int b{ 0 };
    int a{ 255 };
};

struct COMMONHEAD_EXPORT Font
{
    std::string fontFamily;
    int fontSize{ 12 };
    int fontWeight{ 400 };
    bool isItalic{ false };
};
}