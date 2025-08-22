#include <UIResourceColorLoader/UIResourceColorLoader.h>
#include <QQmlEngine>

#include <commonHead/ResourceColorLoader/model/Color.h>
#include <UIResourceColorLoader/UIColorState.h>
#include <UIColorToken.h>

namespace UIResouce{
void UIResourceColorLoader::registerMetaObject()
{
    qmlRegisterUncreatableMetaObject(
	    UIColorToken::staticMetaObject,      // The meta-object of the namespace
	    "UIResourceLoader",                      // The URI or module name
	    1, 0,                          // Version
	    "UIColorToken",                      // The name used in QML
	    "Access to enums only"         // Error message for attempting to create an instance
	);

    qmlRegisterUncreatableMetaObject(
	    UIColorState::staticMetaObject,      // The meta-object of the namespace
	    "UIResourceLoader",                      // The URI or module name
	    1, 0,                          // Version
	    "UIColorState",                      // The name used in QML
	    "Access to enums only"         // Error message for attempting to create an instance
	);
}


commonHead::model::ColorState UIResourceColorLoader::convertUIColoStateToVMColorState(UIColorState::ColorState uiColorState)
{
    switch (uiColorState)
    {
    case UIColorState::ColorState::Normal:
        return commonHead::model::ColorState::Normal;
    case UIColorState::ColorState::Hovered:
        return commonHead::model::ColorState::Hovered;
    case UIColorState::ColorState::Pressed:
        return commonHead::model::ColorState::Pressed;
    case UIColorState::ColorState::Disabled:
        return commonHead::model::ColorState::Disabled;
    case UIColorState::ColorState::Focused:
        return commonHead::model::ColorState::Focused;
    case UIColorState::ColorState::Checked:
        return commonHead::model::ColorState::Checked;
    default:
        return commonHead::model::ColorState::Normal;
    }
}

}