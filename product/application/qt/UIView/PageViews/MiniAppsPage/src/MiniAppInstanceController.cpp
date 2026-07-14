#include "MiniAppInstanceController.h"

#include <utility>

#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <commonHead/viewModels/MiniAppRuntimeViewModel/IMiniAppRuntimeViewModel.h>

#include "ViewModelSingalEmitter/MiniAppRuntimeViewModelEmitter.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace MiniAppsPage {

MiniAppInstanceController::MiniAppInstanceController(QString appId, QObject* parent)
    : UIViewController(parent)
    , mEmitter(std::make_shared<UIVMSignalEmitter::MiniAppRuntimeViewModelEmitter>())
    , mAppId(std::move(appId))
{
    UIVIEW_LOG_DEBUG("create MiniAppInstanceController id=" << mAppId.toStdString());
}

MiniAppInstanceController::~MiniAppInstanceController()
{
    UIVIEW_LOG_DEBUG("delete MiniAppInstanceController id=" << mAppId.toStdString());
    if (mViewModel)
    {
        mViewModel->stop();
    }
}

std::uintptr_t MiniAppInstanceController::nativeHostHandle() const
{
    return mViewModel ? mViewModel->nativeHostHandle() : 0;
}

void MiniAppInstanceController::init()
{
    auto ctx = getAppContext();
    if (!ctx)
    {
        UIVIEW_LOG_WARN("MiniAppInstanceController::init appContext is null, id="
                        << mAppId.toStdString());
        return;
    }

    // The view model owns the agent and resolves package/permissions from the id.
    mViewModel = ctx->getViewModelFactory()->createMiniAppRuntimeViewModelInstance();
    if (!mViewModel)
    {
        UIVIEW_LOG_WARN("MiniAppInstanceController::init failed to create view model, id="
                        << mAppId.toStdString());
        return;
    }

    // Relay the runtime's load events as this controller's own signals so the
    // host window can observe them without touching the emitter.
    using Emitter = UIVMSignalEmitter::MiniAppRuntimeViewModelEmitter;
    QObject::connect(mEmitter.get(), &Emitter::signals_onLoadFinished,
                     this, &MiniAppInstanceController::loadFinished);
    QObject::connect(mEmitter.get(), &Emitter::signals_onLoadFailed,
                     this, &MiniAppInstanceController::loadFailed);

    // Register before start() so we do not miss lifecycle events fired during
    // initialization.
    mViewModel->registerCallback(mEmitter);
    mViewModel->initViewModel();
    // Start first: the runtime only exposes its native web-view window after start().
    mViewModel->start(mAppId.toStdString());
    UIVIEW_LOG_INFO("MiniAppInstanceController runtime started, id=" << mAppId.toStdString());

    emit runtimeStarted();
}

} // namespace MiniAppsPage
