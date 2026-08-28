#include "UIStabilityMonitor.h"

#include <QTimer>
#include <commonhead/viewmodels/StabilityViewModel/IStabilityViewModel.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>

#include "LoggerDefine.h"
#include <UIViewModelSignalBridge/emitters/StabilityViewModelEmitter.h>

UIStabilityMonitor::UIStabilityMonitor(AppContext* appContext, QObject* parent)
    : QObject(parent)
    , mAppContext(appContext)
{
}

UIStabilityMonitor::~UIStabilityMonitor()
{
    stop();
}

void UIStabilityMonitor::start()
{
    APPUI_LOG_DEBUG("start heartbeat");
    mStabilityViewModel = mAppContext->getViewModelFactory()->createStabilityViewModelInstance();
    mStabilityViewModelEmitter = std::make_shared<UIViewModelSignalBridge::StabilityViewModelEmitter>();
    mStabilityViewModel->registerCallback(mStabilityViewModelEmitter);
    mStabilityViewModel->initViewModel();

    mHeartbeatTimer = new QTimer(this);
    QObject::connect(mHeartbeatTimer, &QTimer::timeout, this, [this](){
        if (mStabilityViewModel)
        {
            mStabilityViewModel->reportHeartbeat();
        }
    });
    mHeartbeatTimer->start(mStabilityViewModel->getHeartbeatIntervalMs());
    APPUI_LOG_DEBUG("heartbeat started with interval: " << mStabilityViewModel->getHeartbeatIntervalMs() << "ms");
}

void UIStabilityMonitor::stop()
{
    if (mHeartbeatTimer)
    {
        mHeartbeatTimer->stop();
        mHeartbeatTimer = nullptr;
    }
}
