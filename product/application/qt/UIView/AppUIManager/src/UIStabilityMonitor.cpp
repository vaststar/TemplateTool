#include "UIStabilityMonitor.h"

#include <QTimer>
#include <commonHead/viewModels/StabilityViewModel/IStabilityViewModel.h>
#include <UIFabrication/IViewModelFactory.h>
#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"
#include "ViewModelSingalEmitter/StabilityViewModelEmitter.h"

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
    UIVIEW_LOG_DEBUG("start heartbeat");
    mStabilityViewModel = mAppContext->getViewModelFactory()->createStabilityViewModelInstance();
    mStabilityViewModelEmitter = std::make_shared<UIVMSignalEmitter::StabilityViewModelEmitter>();
    mStabilityViewModel->registerCallback(mStabilityViewModelEmitter);

    mHeartbeatTimer = new QTimer(this);
    QObject::connect(mHeartbeatTimer, &QTimer::timeout, this, [this](){
        if (mStabilityViewModel)
        {
            mStabilityViewModel->reportHeartbeat();
        }
    });
    mHeartbeatTimer->start(mStabilityViewModel->getHeartbeatIntervalMs());
    UIVIEW_LOG_DEBUG("heartbeat started with interval: " << mStabilityViewModel->getHeartbeatIntervalMs() << "ms");
}

void UIStabilityMonitor::stop()
{
    if (mHeartbeatTimer)
    {
        mHeartbeatTimer->stop();
        mHeartbeatTimer = nullptr;
    }
}
