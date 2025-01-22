#pragma once

#include <memory>
#include <initializer_list>
#include <QObject>
#include <QString>
#include <QtQml>
#include <QFont>
#include <QColor>

#include <UIDataStruct/UIStrings.h>
#include <UIDataStruct/UIElementData.h>
#include <UIManager/UIManagerExport.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIManager{
class UIManager_EXPORT ThemeManager final: public QObject
{
Q_OBJECT
QML_ELEMENT
public:
    explicit ThemeManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    ~ThemeManager();
public:
    Q_INVOKABLE QColor getUIColor(UIElementData::UIColorEnum colorEnum, UIElementData::UIColorState state);
    Q_INVOKABLE QFont getUIFont(UIElementData::UIFontSize size, UIElementData::UIFontWeight weight = UIElementData::UIFontWeight::UIFontWeight_Normal, bool isItalic = false, UIElementData::UIFontFamily family = UIElementData::UIFontFamily::UIFontFamily_SegoeUI);

    Q_INVOKABLE QString getLocalizedString(UIStrings::LocalizedString stringId);
    Q_INVOKABLE QString getLocalizedStringWithParams(UIStrings::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params);
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
