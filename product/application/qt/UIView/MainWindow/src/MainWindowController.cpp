#include "MainWindow/include/MainWindowController.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>

#include <UIDataStruct/UILanguage.h>
#include <UIFabrication/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIManager/ITranslatorManager.h>
#include <AppContext/AppContext.h>

#include <UIUtilities/PlatformUtils.h>

#include "LoggerDefine/LoggerDefine.h"


#include "MediaCameraView/include/MediaCameraViewController.h"
#include "ContactList/include/ContactListViewController.h"
#include "MainWindowMenuBar/include/MainWindowMenuBarController.h"
#include "MainWindowTitleBar/include/MainWindowTitleBarController.h"
#include "MainWindowFootBar/include/MainWindowFootBarController.h"
#include "MainWindowSideBar/include/MainWindowSideBarController.h"
#include "AppSystemTray/include/AppSystemTrayController.h"



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
    UIVIEW_LOG_DEBUG("");
    mAppContext = appContext;
    assert(mAppContext);

    mMainViewModel = appContext->getViewModelFactory()->createMainWindowViewModelInstance();
    emit controllerInitialized();
    emit visibleChanged();
}

QString MainWindowController::getTitle() const
{
    return QObject::tr("my test window");
}

int MainWindowController::getHeight() const
{
    return 576;
}

int MainWindowController::getWidth() const
{
    return 758;
}

bool MainWindowController::isVisible() const
{
    return mAppContext != nullptr;
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

    mAppContext->getViewFactory()->loadQmlWindow(QStringLiteral("UIView/MediaCameraView/qml/MediaCameraView.qml"), [this](auto controller){
        if (auto mediaController = dynamic_cast<MediaCameraViewController*>(controller))
        {
            mediaController->initializeController(mAppContext);
        }
    });
}

void MainWindowController::testFunc()
{
    UIVIEW_LOG_DEBUG("");
    UIUtilities::PlatformUtils::openLinkInDefaultBrowser("https://www.baidu.com");
}