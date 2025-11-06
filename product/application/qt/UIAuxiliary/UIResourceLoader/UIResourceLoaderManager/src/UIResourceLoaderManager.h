#pragma once

#include <memory>
#include <initializer_list>
#include <QObject>
#include <QString>
#include <QtQml>
#include <QFont>
#include <QColor>

#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIResource{
class UIResourceLoaderManager final: public IUIResourceLoaderManager
{
Q_OBJECT
QML_ELEMENT
QML_UNCREATABLE("Cannot create a UIResourceLoaderManager instance. It should create by C++")
public:
    explicit UIResourceLoaderManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    ~UIResourceLoaderManager();
    UIResourceLoaderManager(const UIResourceLoaderManager&) = delete;
    UIResourceLoaderManager(UIResourceLoaderManager&&) = delete;
    UIResourceLoaderManager& operator=(const UIResourceLoaderManager&) = delete;
    UIResourceLoaderManager& operator=(UIResourceLoaderManager&&) = delete;
public:
    virtual Q_INVOKABLE QColor getUIColor(UIColorToken::ColorToken colorEnum, UIColorState::ColorState state) override;
    virtual Q_INVOKABLE QFont getUIFont(UIFontToken::UIFontSize size, UIFontToken::UIFontWeight weight = UIFontToken::UIFontWeight::UIFontWeight_Normal, bool isItalic = false, UIFontToken::UIFontFamily family = UIFontToken::UIFontFamily::UIFontFamily_SegoeUI) override;

    virtual Q_INVOKABLE QString getNonLocalizedString(UIStringToken::NonLocalizedString stringId) override;
    virtual Q_INVOKABLE QString getLocalizedString(UIStringToken::LocalizedString stringId) override;
    virtual Q_INVOKABLE QString getLocalizedStringWithParams(UIStringToken::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) override;
private:
    void registerTypes();
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
