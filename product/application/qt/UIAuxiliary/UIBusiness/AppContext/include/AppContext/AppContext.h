#pragma once

#include <memory>
#include <QObject>
#include <QPointer>

#include <AppContext/AppContextExport.h>

namespace UIAppCore{
    class UIApplication;
    class UIQmlEngine;
}

namespace UIFabrication{
    class IUIViewFactory;
}

namespace UIManager{
    class IUIManagerProvider;
}

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
    class IViewModelFactory;
}

class AppContext_EXPORT AppContext final: public QObject
{
Q_OBJECT
public:
    explicit AppContext(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    ~AppContext();
    
    AppContext(const AppContext&) = delete;
    AppContext(AppContext&&) = delete;
    AppContext& operator=(const AppContext&) = delete;
    AppContext& operator=(AppContext&&) = delete;

    QPointer<UIFabrication::IUIViewFactory> getViewFactory() const;
    std::shared_ptr<commonHead::viewModels::IViewModelFactory> getViewModelFactory() const;
    QPointer<UIManager::IUIManagerProvider> getManagerProvider() const;
    QPointer<UIAppCore::UIApplication> getApplication() const;
    QPointer<UIAppCore::UIQmlEngine> getQmlEngine() const;
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};