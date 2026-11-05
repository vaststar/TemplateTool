#include "MainWindow/MainWindowController.h"
#include "MainWindow/MainWindowContentPageRegistry.h"

#include <commonhead/viewmodels/MainWindowViewModel/IMainWindowViewModel.h>

#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIQmlUtilities/QmlWindowPropertyResolver.h>
#include <AppContext/AppContext.h>

#include <UIUtilities/UIPlatformUtils.h>

#include "LoggerDefine.h"
#include "UIWindowUtilities/WindowGeometry.h"

#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <QtCore/qt_windows.h>
#endif

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
    UIVIEW_LOG_DEBUG("MainWindowController constructed, address: " << this);
}

MainWindowController::~MainWindowController()
{
    UIVIEW_LOG_DEBUG("MainWindowController destroying, address: " << this);
}

void MainWindowController::init()
{
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

void MainWindowController::onWindowClosing()
{
    UIVIEW_LOG_INFO(
        "Main window close event received; UI teardown starts after this point"
        ", controller address: "
        << this);
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

bool MainWindowController::applyNativeCaption(QWindow* window)
{
#ifdef Q_OS_WIN
    if (!window || !window->isVisible() || !window->handle()
        || window->windowState() == Qt::WindowFullScreen)
    {
        UIVIEW_LOG_WARN("Native caption skipped: window must be visible, created, and outside fullscreen");
        return false;
    }

    const auto flags = window->flags();
    if (!flags.testFlag(Qt::ExpandedClientAreaHint)
        || !flags.testFlag(Qt::CustomizeWindowHint)
        || flags.testFlag(Qt::FramelessWindowHint)
        || flags.testFlag(Qt::WindowTitleHint))
    {
        UIVIEW_LOG_WARN("Native caption skipped: unexpected Qt flags=0x"
            << QString::number(static_cast<quint32>(flags.toInt()), 16).toStdString());
        return false;
    }

    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    SetLastError(ERROR_SUCCESS);
    const auto styleBefore = GetWindowLongPtrW(hwnd, GWL_STYLE);
    const auto readError = GetLastError();
    if ((styleBefore == 0 && readError != ERROR_SUCCESS) || !(styleBefore & WS_THICKFRAME))
    {
        UIVIEW_LOG_WARN("Native caption skipped: cannot read a resizable native frame, style=0x"
            << QString::number(static_cast<quintptr>(styleBefore), 16).toStdString()
            << ", error=" << readError);
        return false;
    }

    const auto desiredStyle = styleBefore | WS_CAPTION;
    if (styleBefore != desiredStyle)
    {
        SetLastError(ERROR_SUCCESS);
        const auto previousStyle = SetWindowLongPtrW(hwnd, GWL_STYLE, desiredStyle);
        const auto writeError = GetLastError();
        if (previousStyle == 0 && writeError != ERROR_SUCCESS)
        {
            UIVIEW_LOG_ERROR("Native caption style update failed, error=" << writeError);
            return false;
        }

        if (!SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE))
        {
            const auto refreshError = GetLastError();
            UIVIEW_LOG_ERROR("Native caption style written but frame refresh failed, error=" << refreshError);
            return false;
        }
    }

    const auto styleAfter = GetWindowLongPtrW(hwnd, GWL_STYLE);
    const bool applied = (styleAfter & (WS_CAPTION | WS_THICKFRAME)) == (WS_CAPTION | WS_THICKFRAME);
    UIVIEW_LOG_INFO("Native caption result: applied=" << applied
        << ", hwnd=0x" << QString::number(reinterpret_cast<quintptr>(hwnd), 16).toStdString()
        << ", qtFlags=0x" << QString::number(static_cast<quint32>(flags.toInt()), 16).toStdString()
        << ", styleBefore=0x" << QString::number(static_cast<quintptr>(styleBefore), 16).toStdString()
        << ", styleAfter=0x" << QString::number(static_cast<quintptr>(styleAfter), 16).toStdString());
    return applied;
#else
    Q_UNUSED(window);
    return false;
#endif
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
