#include "MainWindow/include/MainWindowController.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>

#include <UIDataStruct/UILanguage.h>
#include <UIFabrication/ViewModelFactory.h>
#include <UIFabrication/UIViewFactory.h>
#include <UIManager/UIManagerProvider.h>
#include <UIManager/TranslatorManager.h>
#include <AppContext/AppContext.h>
#include <UICore/CoreQmlEngine.h>

#include <UIUtilities/PlatformUtils.h>

#include "LoggerDefine/LoggerDefine.h"

#include "ContactList/include/ContactListViewController.h"
#include "MainWindow/include/AppMenuBarController.h"

MainWindowController::MainWindowController(QPointer<AppContext> appContext, QObject* parent)
    : CoreController(parent)
    , mAppContext(appContext)
{
    assert(mAppContext);
    appContext->getQmlEngine()->rootContext()->setContextProperty("mainWindowController", this);
    UIVIEW_LOG_DEBUG("create MainWindowController");
    initializeController(appContext);
}

QString MainWindowController::getControllerName() const
{
    return QStringLiteral("MainWindowController");
}

void MainWindowController::initializeController(QPointer<AppContext> appContext)
{
    mMainViewModel = appContext->getViewModelFactory()->createViewModelInstance<commonHead::viewModels::IMainWindowViewModel>();
    emit controllerInitialized();
}

QString MainWindowController::getTitle() const
{
    return QObject::tr("my test window");
}


// void MainWindowController::onContactListLoaded(ContactListViewController* contactListController)
// {
//     contactListController->initializeController(mAppContext);
// }

void MainWindowController::onInitMenuBarController(AppMenuBarController* menuBarController)
{
    menuBarController->initializeController(mAppContext);
}

void MainWindowController::openCamera()
{
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(UILanguage::LanguageType::LanguageType_ENGLISH);
    emit titleChanged();
    //mMainViewModel->openCamera();
}

void MainWindowController::testFunc()
{
    UIVIEW_LOG_DEBUG("");
    UIUtilities::PlatformUtils::openLinkInDefaultBrowser("https://www.baidu.com");
}


void MainWindowController::startAppWindow()
{
    UIVIEW_LOG_DEBUG("start load main qml");

    mAppContext->getViewFactory()->loadQmlWindow(QStringLiteral("UIView/MainWindow/qml/MainWindow.qml"));
    UIVIEW_LOG_DEBUG("finish load main qml");


    AppMenuBarController* menuBarController = new AppMenuBarController(mAppContext, this);
    emit showMenuBar(menuBarController);

}