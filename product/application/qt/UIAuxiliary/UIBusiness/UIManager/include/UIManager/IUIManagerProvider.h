#pragma once

#include <QObject>

#include <UIManager/UIManagerExport.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

namespace UIAppCore{
    class UIApplication;
    class UIQmlEngine;
}

namespace UIResource{
    class IUIResourceLoaderManager;
}

namespace UIManager{
class ITranslatorManager;
class UIManager_EXPORT IUIManagerProvider: public QObject
{
Q_OBJECT
public:
    IUIManagerProvider() = default;
    IUIManagerProvider(const IUIManagerProvider&) = delete;
    IUIManagerProvider(IUIManagerProvider&&) = delete;
    IUIManagerProvider& operator=(const IUIManagerProvider&) = delete;
    IUIManagerProvider& operator=(IUIManagerProvider&&) = delete;
    virtual ~IUIManagerProvider() = default;
public:
    virtual QPointer<ITranslatorManager> getTranslatorManager() const = 0;
    //UIResourceLoaderManager is unique for application and use UIResource namesapace
    virtual QPointer<UIResource::IUIResourceLoaderManager> getUIResourceLoaderManager() const = 0;

    static std::unique_ptr<IUIManagerProvider> createInstance(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
};
}
