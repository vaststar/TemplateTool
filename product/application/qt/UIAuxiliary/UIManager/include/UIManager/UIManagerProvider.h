#pragma once

#include <QObject>

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
class TranslatorManager;
class ThemeManager;
class UIManager_EXPORT UIManagerProvider final: public QObject
{
Q_OBJECT
public:
    explicit UIManagerProvider(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    ~UIManagerProvider();
    QPointer<TranslatorManager> getTranslatorManager() const;
    QPointer<ThemeManager> getThemeManager() const;
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
