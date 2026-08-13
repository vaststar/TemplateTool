#include "MainWindow/MainWindowController.h"
#include "MainWindow/MainWindowContentPageRegistry.h"

#include <commonhead/viewmodels/MainWindowViewModel/IMainWindowViewModel.h>

#include <TranslatorManager/UILanguage.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIQmlUtilities/QmlWindowPropertyResolver.h>
#include <TranslatorManager/ITranslatorManager.h>
#include <AppContext/AppContext.h>

#include <UIUtilities/UIPlatformUtils.h>

#include "LoggerDefine.h"
#include "UIWindowUtilities/WindowGeometry.h"

#include <QCoreApplication>

#include "MediaCameraView/MediaCameraViewController.h"
#include <UIViewModelSignalBridge/emitters/MainWindowViewModelEmitter.h>

#include "AppUpgrade/AppUpgradeController.h"
#include "UIEvents/UIAboutEvent.h"
#include "UIEvents/UIMainWindowEvent.h"
#include "UIEvents/UIScreenChangedEvent.h"

namespace {
// QML resource paths used by this controller.
const QString kMediaCameraViewQml = QStringLiteral("UIView/MediaCameraView/qml/MediaCameraView.qml");
const QString kAboutDialogQml     = QStringLiteral("UIView/PageViews/AboutPage/qml/AboutDialog.qml");
}

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
    listenUIEvents<UIMainWindowEvent, UIAboutEvent, UIScreenChangedEvent>();

    mMainViewModel = getViewModelFactory()->createMainWindowViewModelInstance();
    mMainViewModelEmitter = std::make_shared<UIViewModelSignalBridge::MainWindowViewModelEmitter>();
    mMainViewModel->registerCallback(mMainViewModelEmitter);
    connect(mMainViewModelEmitter.get(), &UIViewModelSignalBridge::MainWindowViewModelEmitter::signals_onActivateMainWindow,
                this, &MainWindowController::activateMainWindow);
    connect(mMainViewModelEmitter.get(), &UIViewModelSignalBridge::MainWindowViewModelEmitter::signals_onLogsPackComplete,
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
    getTranslatorManager()->loadTranslation(UILanguage::LanguageType::LanguageType_ENGLISH);
    emit titleChanged();

    auto win = getViewFactory()->createQmlItemWindow(
        kMediaCameraViewQml);
    if (!win) return;
    if (auto* mediaController = UIUtilities::QmlWindowPropertyResolver::resolveObjectAs<MediaCameraViewController>(
            win, "controller"))
    {
        setupController(mediaController);
        // Standalone entry: default to local camera 0.
        mediaController->openLocalCamera(0);
    }
    UIUtilities::WindowGeometry::centerOnParentWhenShown(win);
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
    Q_UNUSED(controller);
    // Connect child-controller signals here. This hook runs before the child
    // controller's init(), so initialization-time signals cannot be missed.
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
    auto win = getViewFactory()->createQmlWindow(
        kAboutDialogQml);
    if (!win) return;
    if (auto* aboutController = UIUtilities::QmlWindowPropertyResolver::resolveObjectAs<UIViewController>(
            win, "controller"))
    {
        setupController(aboutController);
    }
    UIUtilities::WindowGeometry::centerOnParentWhenShown(win);
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

QRect MainWindowController::fitToScreen(int x, int y, int width, int height) const
{
    return UIUtilities::WindowGeometry::fitRect(QRect(x, y, width, height));
}

bool MainWindowController::event(QEvent* event)
{
    if (event->type() == UIAboutEvent::eventType())
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
    if (event->type() == UIMainWindowEvent::eventType())
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
    if (event->type() == UIScreenChangedEvent::eventType())
    {
        UIVIEW_LOG_DEBUG("UIScreenChangedEvent: ask QML to re-fit window into available screen area");
        emit screenChanged();
        return true;
    }
    return UIViewController::event(event);
}

void MainWindowController::quitApplication()
{
    UIVIEW_LOG_DEBUG("quitApplication");
    QCoreApplication::quit();
}
