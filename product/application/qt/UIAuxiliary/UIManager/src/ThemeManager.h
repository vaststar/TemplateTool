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
#include <UIManager/IThemeManager.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIManager{
class ThemeManager final: public IThemeManager
{
Q_OBJECT
QML_ELEMENT
QML_UNCREATABLE("Cannot create a ThemeManager instance. It should create by C++")
public:
    explicit ThemeManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    ~ThemeManager();
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager(ThemeManager&&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    ThemeManager& operator=(ThemeManager&&) = delete;
public:
    virtual Q_INVOKABLE QColor getUIColor(UIElementData::UIColorEnum colorEnum, UIElementData::UIColorState state) override;
    virtual Q_INVOKABLE QFont getUIFont(UIElementData::UIFontSize size, UIElementData::UIFontWeight weight = UIElementData::UIFontWeight::UIFontWeight_Normal, bool isItalic = false, UIElementData::UIFontFamily family = UIElementData::UIFontFamily::UIFontFamily_SegoeUI) override;

    virtual Q_INVOKABLE QString getLocalizedString(UIStrings::LocalizedString stringId) override;
    virtual Q_INVOKABLE QString getLocalizedStringWithParams(UIStrings::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) override;
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
