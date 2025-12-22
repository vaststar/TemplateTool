#include "AppUIController.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/InvocationViewModel/IInvocationViewModel.h>
#include <commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h>

#include <UIDataStruct/UIDataUtils.h>
#include <UIFabrication/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <AppContext/AppContext.h>
#include <UIManager/ITranslatorManager.h>
#include <UIIPCChannel/UIIPCServer.h>

#include "LoggerDefine/LoggerDefine.h"
#include "MainWindow/include/MainWindowController.h"
#include "ViewModelSingalEmitter/AppUIViewModelEmitter.h"
#include "ViewModelSingalEmitter/InvocationViewModelEmitter.h"


AppUIController::AppUIController(AppContext* appContext, QObject* parent)
    : QObject(parent)
    , mAppContext(appContext)
{
    UIVIEW_LOG_DEBUG("create AppUIController, address: " << this << ", with appContext: " << appContext);
    initializeController();
}

void AppUIController::initializeController()
{
    UIVIEW_LOG_DEBUG("start initializeController AppUIController, address: " << this);
    assert(mAppContext);

    mAppUIViewModel = mAppContext->getViewModelFactory()->createAppUIViewModelInstance();
    mAppUIViewModelEmitter = std::make_shared<UIVMSignalEmitter::AppUIViewModelEmitter>();
    mAppUIViewModel->registerCallback(mAppUIViewModelEmitter);

    QObject::connect(mAppUIViewModelEmitter.get(), &UIVMSignalEmitter::AppUIViewModelEmitter::signals_onShowMainWindow,
                     this, &AppUIController::onShowMainWindow);
    QObject::connect(mAppUIViewModelEmitter.get(), &UIVMSignalEmitter::AppUIViewModelEmitter::signals_onDatabaseInitialized,
                     this, &AppUIController::onDatabaseInitialized);

    UIVIEW_LOG_DEBUG("finish initializeController AppUIController, address: " << this);
}

void AppUIController::startIPCServer()
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

void AppUIController::onShowMainWindow()
{
    UIVIEW_LOG_DEBUG("start load main qml");
    mAppContext->getViewFactory()->loadQmlWindow(QStringLiteral("UIView/MainWindow/qml/MainWindow.qml"), [this](auto controller){
        if (auto mainController = dynamic_cast<MainWindowController*>(controller))
        {
            UIVIEW_LOG_DEBUG("MainWindow.qml load done, see MainWindowController");
            if (mInvocationViewModelEmitter)
            {
                connect(mInvocationViewModelEmitter.get(), &UIVMSignalEmitter::InvocationViewModelEmitter::signals_onActivateMainApp, 
                mainController, &MainWindowController::activateMainWindow);
            }
            // mImpl->getAppContext()->getViewFactory()->installTranslation({});
            // mainController->initializeController(mAppContext);
        }
    });
    UIVIEW_LOG_DEBUG("finish load main qml");
}

void AppUIController::onDatabaseInitialized()
{
    UIVIEW_LOG_DEBUG("1, load translation after database initialized");
    auto clientInfoVM = mAppContext->getViewModelFactory()->createClientInfoViewModelInstance();
    UIVIEW_LOG_DEBUG("get language" << static_cast<int>(clientInfoVM->getApplicationLanguage()));
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(UIDataUtils::convertViewModelLanguageToUILanguage(clientInfoVM->getApplicationLanguage()));

    UIVIEW_LOG_DEBUG("2, load theme after database initialized");
    UIVIEW_LOG_DEBUG("get CurrentTheme" << static_cast<int>(clientInfoVM->getCurrentThemeType()));
}

void AppUIController::startApp()
{
    UIVIEW_LOG_DEBUG("startApp start");
    startIPCServer();
    mAppUIViewModel->initApplication();
    UIVIEW_LOG_DEBUG("startApp finish");

    // auto invocationVM = mAppContext->getViewModelFactory()->createInvocationViewModelInstance();
    // invocationVM->processStartupParameters();
}