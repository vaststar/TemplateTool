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


    // clientInfoVM->setApplicationLanguage(commonHead::viewModels::model::LanguageType::RUSSIAN);

    UIVIEW_LOG_DEBUG("get language" << static_cast<int>(clientInfoVM->getApplicationLanguage()));
    emit controllerInitialized();
}

void AppUIController::startApp()
{
    //1, install language
    auto clientInfoVM = appContext->getViewModelFactory()->createViewModelInstance<commonHead::viewModels::IClientInfoViewModel>();
    UIVIEW_LOG_DEBUG("get language" << static_cast<int>(clientInfoVM->getApplicationLanguage()));
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(UIManager::LanguageType::CHINESE_SIMPLIFIED);

    //2, show start up page
    //load db to see if we need show sign in/up or main window
    
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

// commonHead::viewModels::model::LanguageType ClientInfoViewModel::convertServiceLanguageToModelLanguage(ucf::service::model::LanguageType language) const
// {
//     switch (language)
//     {
//     case ucf::service::model::LanguageType::ENGLISH:
//         return commonHead::viewModels::model::LanguageType::ENGLISH;
//     case ucf::service::model::LanguageType::CHINESE_SIMPLIFIED:
//         return commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED;
//     case ucf::service::model::LanguageType::CHINESE_TRADITIONAL:
//         return commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL;
//     case ucf::service::model::LanguageType::FRENCH:
//         return commonHead::viewModels::model::LanguageType::FRENCH;
//     case ucf::service::model::LanguageType::GERMAN:
//         return commonHead::viewModels::model::LanguageType::GERMAN;
//     case ucf::service::model::LanguageType::ITALIAN:
//         return commonHead::viewModels::model::LanguageType::ITALIAN;
//     case ucf::service::model::LanguageType::SPANISH:
//         return commonHead::viewModels::model::LanguageType::SPANISH;
//     case ucf::service::model::LanguageType::PORTUGUESE:
//         return commonHead::viewModels::model::LanguageType::PORTUGUESE;
//     case ucf::service::model::LanguageType::JAPANESE:
//         return commonHead::viewModels::model::LanguageType::JAPANESE;
//     case ucf::service::model::LanguageType::KOREAN:
//         return commonHead::viewModels::model::LanguageType::KOREAN;
//     case ucf::service::model::LanguageType::RUSSIAN:
//         return commonHead::viewModels::model::LanguageType::RUSSIAN;
//     default:
//         return commonHead::viewModels::model::LanguageType::ENGLISH;
//     }
// }

// ucf::service::model::LanguageType ClientInfoViewModel::convertModelLanguageToServiceLanguage(commonHead::viewModels::model::LanguageType language) const
// {
//     switch (language)
//     {
//     case commonHead::viewModels::model::LanguageType::ENGLISH:
//         return ucf::service::model::LanguageType::ENGLISH;
//     case commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED:
//         return ucf::service::model::LanguageType::CHINESE_SIMPLIFIED;
//     case commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL:
//         return ucf::service::model::LanguageType::CHINESE_TRADITIONAL;
//     case commonHead::viewModels::model::LanguageType::FRENCH:
//         return ucf::service::model::LanguageType::FRENCH;
//     case commonHead::viewModels::model::LanguageType::GERMAN:
//         return ucf::service::model::LanguageType::GERMAN;
//     case commonHead::viewModels::model::LanguageType::ITALIAN:
//         return ucf::service::model::LanguageType::ITALIAN;
//     case commonHead::viewModels::model::LanguageType::SPANISH:
//         return ucf::service::model::LanguageType::SPANISH;
//     case commonHead::viewModels::model::LanguageType::PORTUGUESE:
//         return ucf::service::model::LanguageType::PORTUGUESE;
//     case commonHead::viewModels::model::LanguageType::JAPANESE:
//         return ucf::service::model::LanguageType::JAPANESE;
//     case commonHead::viewModels::model::LanguageType::KOREAN:
//         return ucf::service::model::LanguageType::KOREAN;
//     case commonHead::viewModels::model::LanguageType::RUSSIAN:
//         return ucf::service::model::LanguageType::RUSSIAN;
//     default:
//         return ucf::service::model::LanguageType::ENGLISH;
//     }
// }
