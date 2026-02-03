#pragma once

#include <memory>
#include <QObject>
#include <QPointer>

class AppContext;
class QTimer;

namespace commonHead::viewModels{
    class IStabilityViewModel;
}

namespace UIVMSignalEmitter{
    class StabilityViewModelEmitter;
}

class UIStabilityMonitor : public QObject
{
    Q_OBJECT
public:
    explicit UIStabilityMonitor(AppContext* appContext, QObject* parent = nullptr);
    ~UIStabilityMonitor();

    void start();
    void stop();

private:
    const QPointer<AppContext> mAppContext;
    std::shared_ptr<commonHead::viewModels::IStabilityViewModel> mStabilityViewModel;
    std::shared_ptr<UIVMSignalEmitter::StabilityViewModelEmitter> mStabilityViewModelEmitter;
    QTimer* mHeartbeatTimer = nullptr;
};
