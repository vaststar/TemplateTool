#pragma once

#include <QObject>

#include <UIManager/IUIManagerProvider.h>

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
class UIManagerProvider final: public IUIManagerProvider
{
Q_OBJECT
public:
    explicit UIManagerProvider(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    ~UIManagerProvider();
    virtual QPointer<ITranslatorManager> getTranslatorManager() const override;
    virtual QPointer<ThemeManager> getThemeManager() const override;
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
