#include "MainWindow/include/MainWindowController.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>

#include <UIManager/UILanguage.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIManager/ITranslatorManager.h>
#include <AppContext/AppContext.h>

#include <UIUtilities/UIPlatformUtils.h>

#include "LoggerDefine/LoggerDefine.h"


#include "MediaCameraView/include/MediaCameraViewController.h"
#include "ViewModelSingalEmitter/MainWindowViewModelEmitter.h"

#include "ContactList/include/ContactListViewController.h"

MainWindowController::MainWindowController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create MainWindowController");
}

MainWindowController::~MainWindowController()
{
    UIVIEW_LOG_DEBUG("destroy MainWindowController");
}

void MainWindowController::init()
{
    UIVIEW_LOG_DEBUG("");

    mMainViewModel = getAppContext()->getViewModelFactory()->createMainWindowViewModelInstance();
    mMainViewModelEmitter = std::make_shared<UIVMSignalEmitter::MainWindowViewModelEmitter>();
    mMainViewModel->registerCallback(mMainViewModelEmitter);
    connect(mMainViewModelEmitter.get(), &UIVMSignalEmitter::MainWindowViewModelEmitter::signals_onActivateMainWindow, 
                this, &MainWindowController::activateMainWindow);
    connect(mMainViewModelEmitter.get(), &UIVMSignalEmitter::MainWindowViewModelEmitter::signals_onLogsPackComplete,
                this, &MainWindowController::onLogsPackComplete);
    mMainViewModel->initViewModel();
}

QString MainWindowController::getTitle() const
{
    return QObject::tr("my test window aa");
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

void MainWindowController::openCamera()
{
    getAppContext()->getManagerProvider()->getTranslatorManager()->loadTranslation(UILanguage::LanguageType::LanguageType_ENGLISH);
    emit titleChanged();

    getAppContext()->getViewFactory()->loadQmlWindow(QStringLiteral("UIView/MediaCameraView/qml/MediaCameraView.qml"), [this](auto controller){
        if (auto mediaController = dynamic_cast<MediaCameraViewController*>(controller))
        {
            UIVIEW_LOG_DEBUG("MediaCameraView.qml load done, will start init MediaCameraViewController");
            mediaController->initializeController(getAppContext());
            UIVIEW_LOG_DEBUG("MediaCameraViewController init done, callback end");
        }
    });
}

void MainWindowController::testFunc()
{
    UIVIEW_LOG_DEBUG("");
    UIUtilities::UIPlatformUtils::openLinkInDefaultBrowser("https://www.baidu.com");
}

void MainWindowController::activateMainWindow()
{
    emit activateWindow();
}

void MainWindowController::packLogs()
{
    UIVIEW_LOG_DEBUG("packLogs called");
    if (mMainViewModel)
    {
        mMainViewModel->packApplicationLogs();
    }
}

void MainWindowController::onLogsPackComplete(bool success, const QString& archivePath)
{
    UIVIEW_LOG_DEBUG("onLogsPackComplete called, success: " << success << ", path: " << archivePath.toStdString());
    if (success && !archivePath.isEmpty())
    {
        UIUtilities::UIPlatformUtils::revealFileInFinder(archivePath.toStdString());
    }
}

void MainWindowController::initController(UIViewController* controller)
{
    if (controller)
    {
        if (auto appContext = getAppContext())
        {
            UIVIEW_LOG_DEBUG("start initialize controller from mainwindowController, controllerName:" << controller->getControllerName().toStdString());
            connectSignals(controller);
            controller->initializeController(appContext);
            UIVIEW_LOG_DEBUG("finish initialze controller from mainwindowController, controllerName: " << controller->getControllerName().toStdString());
        }
        else
        {
            UIVIEW_LOG_DEBUG("initialize controller success");
        }
    }
    else
    {
        UIVIEW_LOG_WARN("controller is null");
    }
}


void MainWindowController::connectSignals(UIViewController* controller)
{
    if (auto contactListController = dynamic_cast<ContactListViewController*>(controller))
    {
        UIVIEW_LOG_DEBUG("connectSignals for ContactListViewController");
        // contactListController->buttonClicked();
    }
}

void MainWindowController::componentCompleted()
{
    emit visibleChanged();
}