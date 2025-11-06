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
class UIManagerProvider final: public IUIManagerProvider
{
Q_OBJECT
public:
    explicit UIManagerProvider(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    ~UIManagerProvider();
    UIManagerProvider(const UIManagerProvider&) = delete;
    UIManagerProvider(UIManagerProvider&&) = delete;
    UIManagerProvider& operator=(const UIManagerProvider&) = delete;
    UIManagerProvider& operator=(UIManagerProvider&&) = delete;

    virtual QPointer<ITranslatorManager> getTranslatorManager() const override;
    virtual QPointer<UIResource::IUIResourceLoaderManager> getUIResourceLoaderManager() const override;
private:
    std::unique_ptr<ITranslatorManager> mTranslatorManager;
    std::unique_ptr<UIResource::IUIResourceLoaderManager> mUIResourceLoaderManager;
};
}
