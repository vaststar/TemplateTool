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
// #include "ContactList/include/ContactListViewController.h"
// #include "MainWindowMenuBar/include/MainWindowMenuBarController.h"
// #include "MainWindowTitleBar/include/MainWindowTitleBarController.h"
// #include "MainWindowFootBar/include/MainWindowFootBarController.h"
// #include "MainWindowSideBar/include/MainWindowSideBarController.h"
// #include "AppSystemTray/include/AppSystemTrayController.h"



MainWindowController::MainWindowController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create MainWindowController");
}

void MainWindowController::init()
{
    UIVIEW_LOG_DEBUG("");

    mMainViewModel = getAppContext()->getViewModelFactory()->createMainWindowViewModelInstance();
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
    return getAppContext() != nullptr;
}

void MainWindowController::onContactListLoaded(ContactListViewController* contactListController)
{
    contactListController->initializeController(getAppContext());
}

void MainWindowController::onInitMenuBarController(MainWindowMenuBarController* menuBarController)
{
    menuBarController->initializeController(getAppContext());
}

void MainWindowController::onInitTitleBarController(MainWindowTitleBarController* titleBarController)
{
    titleBarController->initializeController(getAppContext());
}

void MainWindowController::onInitFootBarController(MainWindowFootBarController* footBarController)
{
    footBarController->initializeController(getAppContext());
}

void MainWindowController::onInitSideBarController(MainWindowSideBarController* sideBarController)
{
    sideBarController->initializeController(getAppContext());
}

void MainWindowController::onInitSystemTrayController(AppSystemTrayController* systemTrayController)
{
    systemTrayController->initializeController(getAppContext());
}

void MainWindowController::onInitializeUIViewController(UIViewController* uiViewController)
{
    uiViewController->initializeController(getAppContext());
}

void MainWindowController::openCamera()
{
    getAppContext()->getManagerProvider()->getTranslatorManager()->loadTranslation(UILanguage::LanguageType::LanguageType_ENGLISH);
    emit titleChanged();

    getAppContext()->getViewFactory()->loadQmlWindow(QStringLiteral("UIView/MediaCameraView/qml/MediaCameraView.qml"), [this](auto controller){
        if (auto mediaController = dynamic_cast<MediaCameraViewController*>(controller))
        {
            mediaController->initializeController(getAppContext());
        }
    });
}

void MainWindowController::testFunc()
{
    UIVIEW_LOG_DEBUG("");
    UIUtilities::PlatformUtils::openLinkInDefaultBrowser("https://www.baidu.com");
}