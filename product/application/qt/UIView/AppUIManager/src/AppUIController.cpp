#include "AppUIController.h"

#include <UIFabrication/ViewModelFactory.h>
#include <UIFabrication/UIViewFactory.h>
#include <UIManager/UIManagerProvider.h>
#include <AppContext/AppContext.h>
#include <UIManager/TranslatorManager.h>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h>

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


void AppUIController::initializeController(AppContext* appContext)
{
    UIVIEW_LOG_DEBUG("init app ui controller");
    mAppContext = appContext;
    assert(mAppContext);
    // mViewModel = appContext->getViewModelFactory()->createViewModelInstance<commonHead::viewModels::IAppUIViewModel>();
    // mViewModel->initDatabase();

    auto clientInfoVM = appContext->getViewModelFactory()->createViewModelInstance<commonHead::viewModels::IClientInfoViewModel>();
    UIVIEW_LOG_DEBUG("get language" << static_cast<int>(clientInfoVM->getApplicationLanguage()));

    // clientInfoVM->setApplicationLanguage(commonHead::viewModels::model::LanguageType::RUSSIAN);

    UIVIEW_LOG_DEBUG("get language" << static_cast<int>(clientInfoVM->getApplicationLanguage()));
    emit controllerInitialized();
}

void AppUIController::startApp()
{
    //load db to see if we need show sign in/up or main window
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(UIManager::LanguageType::CHINESE_SIMPLIFIED);
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