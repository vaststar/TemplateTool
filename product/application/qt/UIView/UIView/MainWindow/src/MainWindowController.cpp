#include "MainWindow/include/MainWindowController.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>

#include <UIDataStruct/UILanguage.h>
#include <UIFabrication/IViewModelFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIManager/ITranslatorManager.h>
#include <AppContext/AppContext.h>

#include <UIUtilities/PlatformUtils.h>

#include "LoggerDefine/LoggerDefine.h"


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

void MainWindowController::initializeController(QPointer<AppContext> appContext)
{
    mAppContext = appContext;
    assert(mAppContext);

    mMainViewModel = appContext->getViewModelFactory()->createMainWindowViewModelInstance();
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

void MainWindowController::onInitMenuBarController(MainWindowMenuBarController* menuBarController)
{
    menuBarController->initializeController(mAppContext);
}

void MainWindowController::onInitTitleBarController(MainWindowTitleBarController* titleBarController)
{
    titleBarController->initializeController(mAppContext);
}

void MainWindowController::onInitFootBarController(MainWindowFootBarController* footBarController)
{
    footBarController->initializeController(mAppContext);
}

void MainWindowController::onInitSideBarController(MainWindowSideBarController* sideBarController)
{
    sideBarController->initializeController(mAppContext);
}

void MainWindowController::onInitSystemTrayController(AppSystemTrayController* systemTrayController)
{
    systemTrayController->initializeController(mAppContext);
}

void MainWindowController::openCamera()
{
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(UILanguage::LanguageType::LanguageType_ENGLISH);
    emit titleChanged();
    mMainViewModel->openCamera();
}

void MainWindowController::testFunc()
{
    UIVIEW_LOG_DEBUG("");
    UIUtilities::PlatformUtils::openLinkInDefaultBrowser("https://www.baidu.com");
}