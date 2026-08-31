#include "UIIPCServerHelper.h"

#include <commonhead/viewmodels/InvocationViewModel/IInvocationViewModel.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <UIIPCChannel/UIIPCServer.h>
#include <AppContext/AppContext.h>

#include "LoggerDefine.h"
#include <UIViewModelSignalBridge/emitters/InvocationViewModelEmitter.h>

UIIPCServerHelper::UIIPCServerHelper(AppContext* appContext, QObject* parent)
    : QObject(parent)
    , mAppContext(appContext)
{
}

UIIPCServerHelper::~UIIPCServerHelper()
{
    stop();
}

void UIIPCServerHelper::start()
{
    APPUI_LOG_DEBUG(
        "UIIPCServerHelper startup started, address: " << this);

    constexpr auto IPC_SERVER_NAME = "TemplateTool_IPC_Server";
    mIPCServer = std::make_shared<UIUtilities::UIIPCServer>(IPC_SERVER_NAME);
    mIPCViewModel = mAppContext->getViewModelFactory()->createInvocationViewModelInstance();
    mInvocationViewModelEmitter = std::make_shared<UIViewModelSignalBridge::InvocationViewModelEmitter>();
    mIPCViewModel->registerCallback(mInvocationViewModelEmitter);
    mIPCViewModel->initViewModel();

    mIPCServer->setMessageHandler([wekPtr = std::weak_ptr<commonHead::viewModels::IInvocationViewModel>(mIPCViewModel)](const std::string& ipcMessage){
        if (auto ptr = wekPtr.lock())
        {
            APPUI_LOG_DEBUG(
                "IPC message processing started, message: " << ipcMessage);
            ptr->processCommandMessage(ipcMessage);
            APPUI_LOG_DEBUG("IPC message processing finished");
        }
        else
        {
            APPUI_LOG_WARN(
                "IPC message processing skipped: InvocationViewModel is unavailable");
        }
    });

    mIPCServer->start();

    APPUI_LOG_DEBUG(
        "UIIPCServerHelper startup finished, address: " << this);
}

void UIIPCServerHelper::stop()
{
    APPUI_LOG_DEBUG(
        "UIIPCServerHelper shutdown started, address: " << this);

    if (mIPCServer)
    {
        mIPCServer.reset();
    }
    else
    {
        APPUI_LOG_DEBUG(
            "UIIPCServerHelper IPC server release skipped: server is not available, address: "
            << this);
    }

    APPUI_LOG_DEBUG(
        "UIIPCServerHelper shutdown finished, address: " << this);
}
