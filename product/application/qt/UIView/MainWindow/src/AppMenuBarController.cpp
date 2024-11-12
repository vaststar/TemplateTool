#include "MainWindow/include/AppMenuBarController.h"

#include <UIFabrication/ViewModelFactory.h>
#include <UIFabrication/UIViewFactory.h>
#include <UIManager/UIManagerProvider.h>
#include <UIManager/TranslatorManager.h>
#include <AppContext/AppContext.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>
#include "LoggerDefine/LoggerDefine.h"

#include "ContactList/include/ContactListViewController.h"

AppMenuBarController::AppMenuBarController(QObject* parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create AppMenuBarController");
}

QString AppMenuBarController::getControllerName() const
{
    return "AppMenuBarController";
}


void AppMenuBarController::initializeController(AppContext* appContext)
{
    mAppContext = appContext;
    assert(mAppContext);

    emit controllerInitialized();
}

void AppMenuBarController::onContactListLoaded(ContactListViewController* contactListController)
{
    contactListController->initializeController(mAppContext);
}

void AppMenuBarController::switchLanguage(UIManager::TranslatorManager::LanguageType languageType)
{
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(languageType);
}
