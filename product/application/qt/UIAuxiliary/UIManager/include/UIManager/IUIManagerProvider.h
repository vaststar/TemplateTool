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

namespace UIResource{
    class IUIResourceLoaderManager;
}

namespace UIManager{
class ITranslatorManager;
class UIManager_EXPORT IUIManagerProvider: public QObject
{
Q_OBJECT
public:
    virtual ~IUIManagerProvider() = default;
    virtual QPointer<ITranslatorManager> getTranslatorManager() const = 0;
    //UIResourceLoaderManager is unique for application and use UIResource namesapace
    virtual QPointer<UIResource::IUIResourceLoaderManager> getUIResourceLoaderManager() const = 0;

    static std::unique_ptr<IUIManagerProvider> createInstance(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
};
}
