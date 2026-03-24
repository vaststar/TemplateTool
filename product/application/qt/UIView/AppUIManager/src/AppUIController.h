#pragma once

#include <memory>
#include <QPointer>

#include <UIViewBase/include/UIViewController.h>

namespace commonHead::viewModels{
    class IAppUIViewModel;
}

class AppContext;
class UIIPCServerHelper;
class UIStabilityMonitor;

namespace UIVMSignalEmitter{
    class AppUIViewModelEmitter;
}

class AppUIController : public UIViewController
{
    Q_OBJECT
public:
    explicit AppUIController(QObject* parent = nullptr);
    ~AppUIController();

protected:
    void init() override;

private slots:
    void onAppConfigInitialized();

private:
    void showMainWindow();

private:
    std::shared_ptr<commonHead::viewModels::IAppUIViewModel> mAppUIViewModel;
    std::shared_ptr<UIVMSignalEmitter::AppUIViewModelEmitter> mAppUIViewModelEmitter;
    std::unique_ptr<UIIPCServerHelper> mIPCServerHelper;
    std::unique_ptr<UIStabilityMonitor> mStabilityMonitor;
};
