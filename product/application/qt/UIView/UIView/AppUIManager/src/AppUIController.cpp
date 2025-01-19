#include "AppUIController.h"

#include <UIFabrication/ViewModelFactory.h>
#include <UIFabrication/UIViewFactory.h>
#include <UIManager/UIManagerProvider.h>
#include <AppContext/AppContext.h>
#include <UIManager/TranslatorManager.h>
// #include <UIManager/ThemeManager.h>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h>

#include <UIUtilities/LanguageUtils.h>

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
    mViewModel = appContext->getViewModelFactory()->createViewModelInstance<commonHead::viewModels::IAppUIViewModel>();
    mViewModel->initDatabase();

    initializeUIClient();

    // clientInfoVM->setApplicationLanguage(commonHead::viewModels::model::LanguageType::RUSSIAN);

    //UIVIEW_LOG_DEBUG("get language" << static_cast<int>(clientInfoVM->getApplicationLanguage()));
    // mAppContext->getManagerProvider()->getThemeManager();
    emit controllerInitialized();
}

void AppUIController::initializeUIClient()
{
    //prepare fontSet、colorSet、language
    //1, install language
    auto clientInfoVM = mAppContext->getViewModelFactory()->createViewModelInstance<commonHead::viewModels::IClientInfoViewModel>();
    UIVIEW_LOG_DEBUG("get language" << static_cast<int>(clientInfoVM->getApplicationLanguage()));
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(LanguageUtils::convertViewModelLanguageToUILanguage(clientInfoVM->getApplicationLanguage()));

    //2, initialize themeManager
    UIVIEW_LOG_DEBUG("get CurrentTheme" << static_cast<int>(clientInfoVM->getCurrentThemeType()));

}

void AppUIController::startApp()
{
    
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