#pragma once

#include <memory>
#include <QObject>
#include <QPointer>

class AppContext;

namespace commonHead::viewModels{
    class IInvocationViewModel;
}

namespace UIVMSignalEmitter{
    class InvocationViewModelEmitter;
}

namespace UIUtilities{
    class UIIPCServer;
}

class UIIPCServerHelper : public QObject
{
    Q_OBJECT
public:
    explicit UIIPCServerHelper(AppContext* appContext, QObject* parent = nullptr);
    ~UIIPCServerHelper();

    void start();
    void stop();

private:
    const QPointer<AppContext> mAppContext;
    std::shared_ptr<UIUtilities::UIIPCServer> mIPCServer;
    std::shared_ptr<commonHead::viewModels::IInvocationViewModel> mIPCViewModel;
    std::shared_ptr<UIVMSignalEmitter::InvocationViewModelEmitter> mInvocationViewModelEmitter;
};
