#pragma once

#include <memory>
#include <QObject>
#include <QPointer>

namespace commonHead::viewModels{
    class IAppUIViewModel;
}

class AppContext;
class UIIPCServerHelper;
class UIStabilityMonitor;

namespace UIVMSignalEmitter{
    class AppUIViewModelEmitter;
}

class AppUIController : public QObject
{
    Q_OBJECT
public:
    AppUIController(AppContext* appContext, QObject* parent = nullptr);
    ~AppUIController();

    void startApp();
private slots:
    void onShowMainWindow();
    void onDatabaseInitialized();
private:
    void initializeController();
private:
    const QPointer<AppContext> mAppContext;
    std::shared_ptr<commonHead::viewModels::IAppUIViewModel> mAppUIViewModel;
    std::shared_ptr<UIVMSignalEmitter::AppUIViewModelEmitter> mAppUIViewModelEmitter;
    std::unique_ptr<UIIPCServerHelper> mIPCServerHelper;
    std::unique_ptr<UIStabilityMonitor> mStabilityMonitor;
};