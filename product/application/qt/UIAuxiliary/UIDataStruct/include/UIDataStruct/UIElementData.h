#pragma once

#include <QObject>
#include <UIDataStruct/UIDataStructExport.h>

namespace UIElementData{
Q_NAMESPACE_EXPORT(UIDataStruct_EXPORT)

UIDataStruct_EXPORT void registerMetaObject();

enum class UIFontFamily{
    UIFontFamily_SegoeUI,
    UIFontFamily_Consolas,
    UIFontFamily_SegoeUIEmoji
};
Q_ENUM_NS(UIFontFamily)

enum class UIFontSize{
    UIFontSize_ExtraSmall,     //12
    UIFontSize_Small,          //14
    UIFontSize_MediumSmall,    //16
    UIFontSize_Medium,         //20
    UIFontSize_Large,          //26
    UIFontSize_ExtraLarge      //40
};
Q_ENUM_NS(UIFontSize)

enum class UIFontWeight{
    UIFontWeight_Thin,
    UIFontWeight_ExtraLight,
    UIFontWeight_Light,
    UIFontWeight_Normal,
    UIFontWeight_Medium,
    UIFontWeight_DemiBold,
    UIFontWeight_Bold,
    UIFontWeight_ExtraBold,
    UIFontWeight_Black
};
Q_ENUM_NS(UIFontWeight)

enum class UIColorState{
    UIColorState_Normal,
    UIColorState_Hovered,
    UIColorState_Pressed,
    UIColorState_Disabled,
    UIColorState_Focused,
    UIColorState_Checked
};
Q_ENUM_NS(UIColorState)

enum class UIColorEnum{
    UIColorEnum_Button_Primary_Text,
    UIColorEnum_Button_Primary_Background,
    UIColorEnum_Button_Primary_Border
};
Q_ENUM_NS(UIColorEnum)
}