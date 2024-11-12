#pragma once

#include <memory>
#include <QObject>
#include <QPointer>

#include <AppContext/AppContextExport.h>

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIFabrication {
    class  UIViewFactory;
    class ViewModelFactory;
}
namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class AppContext_EXPORT AppContext final: public QObject
{
Q_OBJECT
public:
    explicit AppContext(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    ~AppContext();
    
    AppContext(const AppContext&) = delete;
    AppContext(AppContext&&) = delete;
    AppContext& operator=(const AppContext&) = delete;
    AppContext& operator=(AppContext&&) = delete;

    const std::unique_ptr<UIFabrication::UIViewFactory>& getViewFactory() const;
    const std::unique_ptr<UIFabrication::ViewModelFactory>& getViewModelFactory() const;
    QPointer<UICore::CoreApplication> getApplication() const;
    QPointer<UICore::CoreQmlEngine> getQmlEngine() const;
private:
    const std::unique_ptr<UIFabrication::UIViewFactory> mViewFactory;
    const std::unique_ptr<UIFabrication::ViewModelFactory> mViewModelFactory;
    const QPointer<UICore::CoreApplication> mApplication;
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
};