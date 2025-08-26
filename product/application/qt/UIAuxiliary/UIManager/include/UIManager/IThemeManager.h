#pragma once

#include <memory>
#include <initializer_list>
#include <QObject>
#include <QString>
#include <QtQml>
#include <QFont>
#include <QColor>

#include <UIDataStruct/UIElementData.h>
#include <UIManager/UIManagerExport.h>
#include <UIResourceColorLoader/UIColorState.h>
#include <UIColorToken.h>
#include <UIStringToken.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIManager{
class UIManager_EXPORT IThemeManager: public QObject
{
Q_OBJECT
public:
    virtual ~IThemeManager() = default;
public:
    virtual Q_INVOKABLE QColor getUIColor(UIColorToken::ColorToken colorEnum, UIColorState::ColorState state) = 0;
    virtual Q_INVOKABLE QFont getUIFont(UIElementData::UIFontSize size, UIElementData::UIFontWeight weight = UIElementData::UIFontWeight::UIFontWeight_Normal, bool isItalic = false, UIElementData::UIFontFamily family = UIElementData::UIFontFamily::UIFontFamily_SegoeUI) = 0;

    virtual Q_INVOKABLE QString getLocalizedString(UIStringToken::LocalizedString stringId) = 0;
    virtual Q_INVOKABLE QString getLocalizedStringWithParams(UIStringToken::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) = 0;
};
}
