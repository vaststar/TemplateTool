#include <UIResourceFontLoader/UIResourceFontLoader.h>

#include <QQmlEngine>

#include <commonHead/ResourceFontLoader/ResourceFontModel.h>
#include <UIResourceFontLoader/UIFontToken.h>

#include "LoggerDefine.h"

namespace UIResource{

void UIResourceFontLoader::registerUIResourceFontLoader()
{
    UIResourceFontLoader_LOG_DEBUG("");
    qmlRegisterUncreatableMetaObject(
	    UIFontToken::staticMetaObject,      // The meta-object of the namespace
	    "UIResourceLoader",                      // The URI or module name
	    1, 0,                          // Version
	    "UIFontToken",                      // The name used in QML
	    "Access to enums only"         // Error message for attempting to create an instance
	);
}

commonHead::model::FontFamily UIResourceFontLoader::convertUIFontFamilyToVMFontFamily(UIFontToken::UIFontFamily uiFontFamily)
{
    switch (uiFontFamily)
    {
    case UIFontToken::UIFontFamily::UIFontFamily_SegoeUI:
        return commonHead::model::FontFamily::SegoeUI;
    case UIFontToken::UIFontFamily::UIFontFamily_Consolas:
        return commonHead::model::FontFamily::Consolas;
    case UIFontToken::UIFontFamily::UIFontFamily_SegoeUIEmoji:
        return commonHead::model::FontFamily::SegoeUIEmoji;
    default:
        return commonHead::model::FontFamily::SegoeUI;
    }
}

commonHead::model::FontSize UIResourceFontLoader::convertUIFontSizeToVMFontSize(UIFontToken::UIFontSize uiFontSize)
{
    switch (uiFontSize)
    {
    case UIFontToken::UIFontSize::UIFontSize_ExtraSmall:
        return commonHead::model::FontSize::ExtraSmall;
    case UIFontToken::UIFontSize::UIFontSize_Small:
        return commonHead::model::FontSize::Small;
    case UIFontToken::UIFontSize::UIFontSize_MediumSmall:
        return commonHead::model::FontSize::MediumSmall;
    case UIFontToken::UIFontSize::UIFontSize_Medium:
        return commonHead::model::FontSize::Medium;
    case UIFontToken::UIFontSize::UIFontSize_Large:
        return commonHead::model::FontSize::Large;
    case UIFontToken::UIFontSize::UIFontSize_ExtraLarge:
        return commonHead::model::FontSize::ExtraLarge;
    default:
        return commonHead::model::FontSize::Small;
    }
}

commonHead::model::FontWeight UIResourceFontLoader::convertUIFontWeightToVMFontWeight(UIFontToken::UIFontWeight uiFontWeight)
{
    switch (uiFontWeight)
    {
    case UIFontToken::UIFontWeight::UIFontWeight_Thin:
        return commonHead::model::FontWeight::Thin;
    case UIFontToken::UIFontWeight::UIFontWeight_ExtraLight:
        return commonHead::model::FontWeight::ExtraLight;
    case UIFontToken::UIFontWeight::UIFontWeight_Light:
        return commonHead::model::FontWeight::Light;
    case UIFontToken::UIFontWeight::UIFontWeight_Normal:
        return commonHead::model::FontWeight::Normal;
    case UIFontToken::UIFontWeight::UIFontWeight_Medium:
        return commonHead::model::FontWeight::Medium;
    case UIFontToken::UIFontWeight::UIFontWeight_DemiBold:
        return commonHead::model::FontWeight::DemiBold;
    case UIFontToken::UIFontWeight::UIFontWeight_Bold:
        return commonHead::model::FontWeight::Bold;
    case UIFontToken::UIFontWeight::UIFontWeight_ExtraBold:
        return commonHead::model::FontWeight::ExtraBold;
    case UIFontToken::UIFontWeight::UIFontWeight_Black:
        return commonHead::model::FontWeight::Black;
    default:
        return commonHead::model::FontWeight::Normal;
    }
}
}