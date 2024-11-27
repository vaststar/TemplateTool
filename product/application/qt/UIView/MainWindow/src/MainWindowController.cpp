#include "MainWindow/include/MainWindowController.h"

#include <UIFabrication/ViewModelFactory.h>
#include <UIFabrication/UIViewFactory.h>
#include <UIManager/UIManagerProvider.h>
#include <UIManager/TranslatorManager.h>
#include <AppContext/AppContext.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>
#include "LoggerDefine/LoggerDefine.h"

#include "ContactList/include/ContactListViewController.h"
#include "MainWindow/include/AppMenuBarController.h"

MainWindowController::MainWindowController(QObject* parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create MainWindowController");
}

QString MainWindowController::getControllerName() const
{
    return QStringLiteral("MainWindowController");
}


void MainWindowController::initializeController(AppContext* appContext)
{
    mAppContext = appContext;
    assert(mAppContext);

    mMainViewModel = appContext->getViewModelFactory()->createViewModelInstance<commonHead::viewModels::IMainWindowViewModel>();
    mMainViewModel->initDatabase();
    emit controllerInitialized();
}

QString MainWindowController::getTitle() const
{
    return QObject::tr("my test window");
}


void MainWindowController::onContactListLoaded(ContactListViewController* contactListController)
{
    contactListController->initializeController(mAppContext);
}

void MainWindowController::onInitMenuBarController(AppMenuBarController* menuBarController)
{
    menuBarController->initializeController(mAppContext);
}

void MainWindowController::openCamera()
{
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(UIManager::LanguageType::CHINESE_SIMPLIFIED);
    emit titleChanged();
    //mMainViewModel->openCamera();
}
