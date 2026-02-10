#include "AppUIController.h"

#include <commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h>

#include <UIManager/UILanguage.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <AppContext/AppContext.h>
#include <UIManager/ITranslatorManager.h>

#include "LoggerDefine/LoggerDefine.h"
#include "MainWindow/include/MainWindowController.h"
#include "ViewModelSingalEmitter/AppUIViewModelEmitter.h"
#include "UIIPCServerHelper.h"
#include "UIStabilityMonitor.h"


AppUIController::AppUIController(AppContext* appContext, QObject* parent)
    : QObject(parent)
    , mAppContext(appContext)
{
    UIVIEW_LOG_DEBUG("create AppUIController, address: " << this << ", with appContext: " << appContext);
    initializeController();
}

AppUIController::~AppUIController() = default;

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

void AppUIController::onShowMainWindow()
{
    UIVIEW_LOG_DEBUG("start load main qml");
    mAppContext->getViewFactory()->loadQmlWindow(QStringLiteral("UIView/MainWindow/qml/MainWindow.qml"), [this](auto controller){
        if (auto mainController = dynamic_cast<MainWindowController*>(controller))
        {
            UIVIEW_LOG_DEBUG("MainWindow.qml load done, will start init MainWindowController");
            mainController->initializeController(mAppContext);
            UIVIEW_LOG_DEBUG("MainWindowController init done, callback end");
        }
    });
    UIVIEW_LOG_DEBUG("finish load main qml");
}

void AppUIController::onDatabaseInitialized()
{
    UIVIEW_LOG_DEBUG("1, load translation after database initialized");
    auto clientInfoVM = mAppContext->getViewModelFactory()->createClientInfoViewModelInstance();
    UIVIEW_LOG_DEBUG("get language" << static_cast<int>(clientInfoVM->getApplicationLanguage()));
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(UILanguage::convertFromViewModel(clientInfoVM->getApplicationLanguage()));

    UIVIEW_LOG_DEBUG("2, load theme after database initialized");
    UIVIEW_LOG_DEBUG("get CurrentTheme" << static_cast<int>(clientInfoVM->getCurrentThemeType()));
}

void AppUIController::startApp()
{
    UIVIEW_LOG_DEBUG("startApp start");
    mIPCServerHelper = std::make_unique<UIIPCServerHelper>(mAppContext, this);
    mIPCServerHelper->start();

    mStabilityMonitor = std::make_unique<UIStabilityMonitor>(mAppContext, this);
    mStabilityMonitor->start();

    mAppUIViewModel->initApplication();
    UIVIEW_LOG_DEBUG("startApp finish");
}