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

#include "LoggerDefine/LoggerDefine.h"
#include "MainWindow/include/MainWindowController.h"


AppUIController::AppUIController(QObject* parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create AppUIController");
}

QString AppUIController::getControllerName() const
{
    return "AppUIController";
}

void AppUIController::runApp(AppContext* appContext)
{
    initializeController(appContext);
    startApp();
}

void AppUIController::initializeController(AppContext* appContext)
{
    UIVIEW_LOG_DEBUG("init app ui controller");
    mAppContext = appContext;
    assert(mAppContext);
    mViewModel = appContext->getViewModelFactory()->createAppUIViewModelInstance();
    mViewModel->initApplication();

    initializeUIClient();

    emit controllerInitialized();
}

void AppUIController::initializeUIClient()
{
    //prepare fontSet、colorSet、language
    //1, install language
    auto clientInfoVM = mAppContext->getViewModelFactory()->createClientInfoViewModelInstance();
    UIVIEW_LOG_DEBUG("get language" << static_cast<int>(clientInfoVM->getApplicationLanguage()));
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(UIDataUtils::convertViewModelLanguageToUILanguage(clientInfoVM->getApplicationLanguage()));

    //2, initialize themeManager
    UIVIEW_LOG_DEBUG("get CurrentTheme" << static_cast<int>(clientInfoVM->getCurrentThemeType()));

}

void AppUIController::startApp()
{
    auto invocationVM = mAppContext->getViewModelFactory()->createInvocationViewModelInstance();
    invocationVM->processStartupParameters();

    UIVIEW_LOG_DEBUG("start load main qml");

    mAppContext->getViewFactory()->loadQmlWindow(QStringLiteral("UIView/MainWindow/qml/MainWindow.qml"), "MainWindowController", [this](auto controller){
        if (auto mainController = dynamic_cast<MainWindowController*>(controller))
        {
            // mImpl->getAppContext()->getViewFactory()->installTranslation({});
            mainController->initializeController(mAppContext);
        }
    });
    UIVIEW_LOG_DEBUG("finish load main qml");

}