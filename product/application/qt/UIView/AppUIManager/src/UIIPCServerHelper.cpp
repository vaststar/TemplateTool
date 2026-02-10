#include "UIIPCServerHelper.h"

#include <commonHead/viewModels/InvocationViewModel/IInvocationViewModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIIPCChannel/UIIPCServer.h>
#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"
#include "ViewModelSingalEmitter/InvocationViewModelEmitter.h"

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
    UIVIEW_LOG_DEBUG("start ipc server");
    constexpr auto IPC_SERVER_NAME = "TemplateTool_IPC_Server";
    mIPCServer = std::make_shared<UIUtilities::UIIPCServer>(IPC_SERVER_NAME);
    mIPCViewModel = mAppContext->getViewModelFactory()->createInvocationViewModelInstance();
    mInvocationViewModelEmitter = std::make_shared<UIVMSignalEmitter::InvocationViewModelEmitter>();
    mIPCViewModel->registerCallback(mInvocationViewModelEmitter);

    mIPCServer->setMessageHandler([wekPtr = std::weak_ptr<commonHead::viewModels::IInvocationViewModel>(mIPCViewModel)](const std::string& ipcMessage){
        if (auto ptr = wekPtr.lock()) 
        {
            UIVIEW_LOG_DEBUG("start process new message: " << ipcMessage);
            ptr->processCommandMessage(ipcMessage);
            UIVIEW_LOG_DEBUG("finish process message");
        }
        else
        {
            UIVIEW_LOG_INFO("no InvocationViewModel");
        }
    });
    mIPCServer->start();
    UIVIEW_LOG_DEBUG("start ipc server succeed");
}

void UIIPCServerHelper::stop()
{
    if (mIPCServer)
    {
        mIPCServer.reset();
    }
}
