#include "MainWindow/include/MainWindowController.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>

#include <UIDataStruct/UILanguage.h>
#include <UIFabrication/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIManager/ITranslatorManager.h>
#include <AppContext/AppContext.h>

#include <UIUtilities/UIPlatformUtils.h>

#include "LoggerDefine/LoggerDefine.h"


#include "MediaCameraView/include/MediaCameraViewController.h"
#include "ViewModelSingalEmitter/MainWindowViewModelEmitter.h"

MainWindowController::MainWindowController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create MainWindowController");
}

MainWindowController::~MainWindowController()
{
    UIVIEW_LOG_DEBUG("delete MainWindowController");
    // if (mMainViewModel && mMainViewModelEmitter)
    // {
    //     mMainViewModel->unRegisterCallback(mMainViewModelEmitter);
    // }
}

void MainWindowController::init()
{
    UIVIEW_LOG_DEBUG("");

    mMainViewModel = getAppContext()->getViewModelFactory()->createMainWindowViewModelInstance();
    mMainViewModelEmitter = std::make_shared<UIVMSignalEmitter::MainWindowViewModelEmitter>();
    mMainViewModel->registerCallback(mMainViewModelEmitter);
    connect(mMainViewModelEmitter.get(), &UIVMSignalEmitter::MainWindowViewModelEmitter::signals_onActivateMainWindow, 
                this, &MainWindowController::activateMainWindow);
    // connect(mMainViewModelEmitter.get(), &QObject::destroyed, this, [this]() {
    //         if (mMainViewModel)
    //         {
    //             mMainViewModel->unRegisterCallback(mMainViewModelEmitter);
    //         }
    // });
    mMainViewModel->initViewModel();
    emit controllerInitialized();
    emit visibleChanged();
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
            // mediaController->initializeController(getAppContext());
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
