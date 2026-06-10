#include "MainWindow/MainWindowController.h"
#include "MainWindow/MainWindowContentPageRegistry.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>

#include <TranslatorManager/UILanguage.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <TranslatorManager/ITranslatorManager.h>
#include <AppContext/AppContext.h>

#include <UIUtilities/UIPlatformUtils.h>

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
#include "UIViewHelper/UIViewHelper.h"

#include <QCoreApplication>

#include "MediaCameraView/MediaCameraViewController.h"
#include "ViewModelSingalEmitter/MainWindowViewModelEmitter.h"

#include "ContactsPage/ContactsPageController.h"
#include "HomePage/HomePageController.h"
#include "SettingsPage/SettingsPageController.h"
#include "AppUpgrade/AppUpgradeController.h"
#include "AboutPage/AboutPageController.h"
#include "UIEvents/UIAboutEvent.h"
#include "UIEvents/UIMainWindowEvent.h"

MainWindowController::MainWindowController(QObject* parent)
    : UIViewController(parent)
    , m_pageRegistry(new MainWindowContentPageRegistry(this))
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

    // Listen for events from EventBus
    listenUIEvents<UIMainWindowEvent, UIAboutEvent>();

    mMainViewModel = getAppContext()->getViewModelFactory()->createMainWindowViewModelInstance();
    mMainViewModelEmitter = std::make_shared<UIVMSignalEmitter::MainWindowViewModelEmitter>();
    mMainViewModel->registerCallback(mMainViewModelEmitter);
    connect(mMainViewModelEmitter.get(), &UIVMSignalEmitter::MainWindowViewModelEmitter::signals_onActivateMainWindow,
                this, &MainWindowController::activateMainWindow);
    connect(mMainViewModelEmitter.get(), &UIVMSignalEmitter::MainWindowViewModelEmitter::signals_onLogsPackComplete,
                this, &MainWindowController::onLogsPackComplete);

    mMainViewModel->initViewModel();

    createUpgradeController();
}

QString MainWindowController::getTitle() const
{
    return QObject::tr("my test window aa");
}

bool MainWindowController::isVisible() const
{
    return getAppContext() != nullptr;
}

void MainWindowController::openCamera()
{
    getAppContext()->getManagerProvider()->getTranslatorManager()->loadTranslation(UILanguage::LanguageType::LanguageType_ENGLISH);
    emit titleChanged();

    auto win = getAppContext()->getViewFactory()->createQmlItemWindow(
        QStringLiteral("UIView/MediaCameraView/qml/MediaCameraView.qml"));
    if (!win) return;
    if (auto* mediaController = UIView::UIViewHelper::controllerOf<MediaCameraViewController>(win))
    {
        mediaController->initializeController(getAppContext());
        // Standalone entry: default to local camera 0.
        mediaController->openLocalCamera(0);
    }
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
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

void MainWindowController::onSetupController(UIViewController* controller)
{
    connectSignals(controller);
}

void MainWindowController::connectSignals(UIViewController* controller)
{
    if (dynamic_cast<ContactsPageController*>(controller))
    {
        UIVIEW_LOG_DEBUG("connectSignals for ContactsPageController");
    }
    else if (dynamic_cast<HomePageController*>(controller))
    {
        UIVIEW_LOG_DEBUG("connectSignals for HomePageController");
    }
    else if (dynamic_cast<SettingsPageController*>(controller))
    {
        UIVIEW_LOG_DEBUG("connectSignals for SettingsPageController");
    }
}

void MainWindowController::createUpgradeController()
{
    if (!m_upgradeController)
    {
        UIVIEW_LOG_DEBUG("Creating AppUpgradeController");
        m_upgradeController = new AppUpgradeController(this);
        connect(m_upgradeController, &AppUpgradeController::quitRequested, this, &MainWindowController::quitApplication);
        setupController(m_upgradeController);
    }
}

void MainWindowController::showAboutDialog()
{
    UIVIEW_LOG_DEBUG("showAboutDialog");
    auto win = getAppContext()->getViewFactory()->createQmlWindow(
        QStringLiteral("UIView/PageViews/AboutPage/qml/AboutDialog.qml"));
    if (!win) return;
    if (auto* aboutController = UIView::UIViewHelper::controllerOf<UIViewController>(win))
    {
        setupController(aboutController);
    }
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
}

void MainWindowController::componentCompleted()
{
    emit visibleChanged();
}

MainWindowContentPageRegistry* MainWindowController::pageRegistry() const
{
    return m_pageRegistry;
}

bool MainWindowController::startSystemResize(QWindow *window, int edges)
{
    if (window)
    {
        return window->startSystemResize(Qt::Edges(edges));
    }
    return false;
}

bool MainWindowController::event(QEvent* event)
{
    if (event->type() == UIAboutEvent::type)
    {
        switch (static_cast<UIAboutEvent*>(event)->mAction)
        {
        case UIAboutEvent::Action::ShowAboutDialog:
            UIVIEW_LOG_DEBUG("UIAboutEvent::ShowAboutDialog");
            showAboutDialog();
            break;
        default:
            UIVIEW_LOG_DEBUG("UIAboutEvent::UnknownAction");
            break;
        }
        return true;
    }
    if (event->type() == UIMainWindowEvent::type)
    {
        switch (static_cast<UIMainWindowEvent*>(event)->mAction)
        {
        case UIMainWindowEvent::Action::Hide:
            UIVIEW_LOG_DEBUG("UIMainWindowEvent::Hide");
            emit hideWindow();
            break;
        case UIMainWindowEvent::Action::Show:
            UIVIEW_LOG_DEBUG("UIMainWindowEvent::Show");
            emit showWindow();
            break;
        case UIMainWindowEvent::Action::Activate:
            UIVIEW_LOG_DEBUG("UIMainWindowEvent::Activate");
            emit activateWindow();
            break;
        case UIMainWindowEvent::Action::Minimize:
            UIVIEW_LOG_DEBUG("UIMainWindowEvent::Minimize");
            emit minimizeWindow();
            break;
        case UIMainWindowEvent::Action::Maximize:
            UIVIEW_LOG_DEBUG("UIMainWindowEvent::Maximize");
            break;
        case UIMainWindowEvent::Action::Restore:
            UIVIEW_LOG_DEBUG("UIMainWindowEvent::Restore");
            emit showWindow();
            break;
        case UIMainWindowEvent::Action::Close:
            UIVIEW_LOG_DEBUG("UIMainWindowEvent::Close");
            quitApplication();
            break;
        default:
            UIVIEW_LOG_DEBUG("UIMainWindowEvent::UnknownAction");
            break;
        }
        return true;
    }
    return UIViewController::event(event);
}

void MainWindowController::quitApplication()
{
    UIVIEW_LOG_DEBUG("quitApplication");
    QCoreApplication::quit();
}
