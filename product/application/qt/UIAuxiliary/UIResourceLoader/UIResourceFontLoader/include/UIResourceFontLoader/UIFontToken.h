#pragma once

#include <QObject>
#include <UIResourceFontLoader/UIResourceFontLoaderExport.h>

namespace UIFontToken{
Q_NAMESPACE_EXPORT(UIResourceFontLoader_EXPORT)
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
}