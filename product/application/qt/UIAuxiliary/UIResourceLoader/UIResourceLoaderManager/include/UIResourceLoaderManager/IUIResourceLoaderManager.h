#pragma once

#include <initializer_list>
#include <QObject>
#include <QString>
#include <QFont>
#include <QColor>

#include <UIResourceLoaderManager/UIResourceLoaderManagerExport.h>

#include <UIResourceColorLoader/UIColorState.h>
#include <UIColorToken.h>
#include <UIStringToken.h>
#include <UIFontToken.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIResource{
class UIResourceLoaderManager_EXPORT IUIResourceLoaderManager: public QObject
{
Q_OBJECT
public:
    IUIResourceLoaderManager() = default;
    IUIResourceLoaderManager(const IUIResourceLoaderManager&) = delete;
    IUIResourceLoaderManager(IUIResourceLoaderManager&&) = delete;
    IUIResourceLoaderManager& operator=(const IUIResourceLoaderManager&) = delete;
    IUIResourceLoaderManager& operator=(IUIResourceLoaderManager&&) = delete;
    virtual ~IUIResourceLoaderManager() = default;
public:
    virtual Q_INVOKABLE QColor getUIColor(UIColorToken::ColorToken colorEnum, UIColorState::ColorState state) = 0;
    virtual Q_INVOKABLE QFont getUIFont(UIFontToken::FontToken fontToken) = 0;

    virtual Q_INVOKABLE QString getNonLocalizedString(UINonLocalizedStringToken::NonLocalizedString stringId) = 0;
    virtual Q_INVOKABLE QString getLocalizedString(UILocalizedStringToken::LocalizedString stringId) = 0;
    virtual Q_INVOKABLE QString getLocalizedStringWithParams(UILocalizedStringWithParamToken::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) = 0;
public:
    static std::unique_ptr<IUIResourceLoaderManager> createInstance(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
};
}
