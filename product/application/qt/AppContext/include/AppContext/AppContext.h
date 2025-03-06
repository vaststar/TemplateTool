#pragma once

#include <memory>
#include <QObject>
#include <QPointer>

#include <AppContext/AppContextExport.h>

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIFabrication{
    class  IUIViewFactory;
    class ViewModelFactory;
}

namespace UIManager{
    class IUIManagerProvider;
}
namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class AppContext_EXPORT AppContext final: public QObject
{
Q_OBJECT
public:
    explicit AppContext(QPointer<UICore::CoreApplication> application, QPointer<UICore::CoreQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    ~AppContext();
    
    AppContext(const AppContext&) = delete;
    AppContext(AppContext&&) = delete;
    AppContext& operator=(const AppContext&) = delete;
    AppContext& operator=(AppContext&&) = delete;

    QPointer<UIFabrication::IUIViewFactory> getViewFactory() const;
    QPointer<UIFabrication::ViewModelFactory> getViewModelFactory() const;
    QPointer<UIManager::IUIManagerProvider> getManagerProvider() const;
    QPointer<UICore::CoreApplication> getApplication() const;
    QPointer<UICore::CoreQmlEngine> getQmlEngine() const;
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};