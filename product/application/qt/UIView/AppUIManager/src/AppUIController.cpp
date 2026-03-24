#include "AppUIController.h"

#include <commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h>

#include <TranslatorManager/UILanguage.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <AppContext/AppContext.h>
#include <TranslatorManager/ITranslatorManager.h>
#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>

#include <QGuiApplication>
#include <QStyleHints>
#include <QTimer>

#include "LoggerDefine/LoggerDefine.h"
#include "MainWindow/include/MainWindowController.h"
#include "ViewModelSingalEmitter/AppUIViewModelEmitter.h"
#include "UIIPCServerHelper.h"
#include "UIStabilityMonitor.h"


AppUIController::AppUIController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create AppUIController, address: " << this);
}

AppUIController::~AppUIController() = default;

void AppUIController::init()
{
    UIVIEW_LOG_DEBUG("init AppUIController start");
    auto appContext = getAppContext();

    // Create ViewModel and connect signals
    mAppUIViewModel = appContext->getViewModelFactory()->createAppUIViewModelInstance();
    mAppUIViewModelEmitter = std::make_shared<UIVMSignalEmitter::AppUIViewModelEmitter>();
    mAppUIViewModel->registerCallback(mAppUIViewModelEmitter);

    QObject::connect(mAppUIViewModelEmitter.get(), &UIVMSignalEmitter::AppUIViewModelEmitter::signals_onAppConfigInitialized,
                     this, &AppUIController::onAppConfigInitialized);

    // Start IPC server and stability monitor
    mIPCServerHelper = std::make_unique<UIIPCServerHelper>(appContext, this);
    mIPCServerHelper->start();

    mStabilityMonitor = std::make_unique<UIStabilityMonitor>(appContext, this);
    mStabilityMonitor->start();

    // Defer application initialization to the first event loop iteration,
    // ensuring QApplication::exec() is running before async callbacks fire.
    QTimer::singleShot(0, this, [this]() {
        mAppUIViewModel->initApplication();
    });
    UIVIEW_LOG_DEBUG("init AppUIController finish");
}

void AppUIController::onAppConfigInitialized()
{
    UIVIEW_LOG_DEBUG("onAppConfigInitialized start");
    auto appContext = getAppContext();

    // 1. Load translation
    auto clientInfoViewModel = appContext->getViewModelFactory()->createClientInfoViewModelInstance();
    UIVIEW_LOG_DEBUG("get language: " << static_cast<int>(clientInfoViewModel->getApplicationLanguage()));
    appContext->getManagerProvider()->getTranslatorManager()->loadTranslation(
        UILanguage::convertFromViewModel(clientInfoViewModel->getApplicationLanguage()));

    // 2. Load theme
    UIVIEW_LOG_DEBUG("get CurrentTheme: " << static_cast<int>(clientInfoViewModel->getCurrentThemeType()));

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this]() {
        auto clientInfoViewModel = getAppContext()->getViewModelFactory()->createClientInfoViewModelInstance();
        UIVIEW_LOG_DEBUG("system color scheme changed, CurrentTheme: " << static_cast<int>(clientInfoViewModel->getCurrentThemeType()));
        if (clientInfoViewModel->getCurrentThemeType() == commonHead::viewModels::model::ThemeType::SystemDefault)
        {
            UIVIEW_LOG_DEBUG("system color scheme changed, notifying theme refresh");
            getAppContext()->getManagerProvider()->getUIResourceLoaderManager()->notifyThemeChanged();
        }
    });

    // 3. Show main window
    showMainWindow();

    UIVIEW_LOG_DEBUG("onAppConfigInitialized finish");
}

void AppUIController::showMainWindow()
{
    UIVIEW_LOG_DEBUG("start load main qml");
    getAppContext()->getViewFactory()->loadQmlWindow(QStringLiteral("UIView/MainWindow/qml/MainWindow.qml"), [this](auto controller){
        if (auto mainController = dynamic_cast<MainWindowController*>(controller))
        {
            UIVIEW_LOG_DEBUG("MainWindow.qml load done, will start init MainWindowController");
            mainController->initializeController(getAppContext());
            UIVIEW_LOG_DEBUG("MainWindowController init done, callback end");
        }
    });
    UIVIEW_LOG_DEBUG("finish load main qml");
}
