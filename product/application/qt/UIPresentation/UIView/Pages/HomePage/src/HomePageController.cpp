#include "HomePage/HomePageController.h"
#include "LoggerDefine.h"
#include "UIWindowUtilities/WindowGeometry.h"
#include "UIViewMessageBox/UIViewMessageBoxHelper.h"
#include "MediaCameraView/MediaCameraViewController.h"
#include "CameraMonitorView/CameraMonitorViewController.h"

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIQmlUtilities/QmlWindowPropertyResolver.h>

namespace {
// QML resource paths used by this controller.
const QString kMediaCameraViewQml   = QStringLiteral("UIView/MediaCameraView/qml/MediaCameraView.qml");
const QString kCameraMonitorViewQml = QStringLiteral("UIView/CameraMonitorView/qml/CameraMonitorView.qml");
}

HomePageController::HomePageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("HomePageController constructed, address: " << this);
}

HomePageController::~HomePageController()
{
    UIVIEW_LOG_DEBUG("HomePageController destroying, address: " << this);
}

void HomePageController::init()
{
    UIVIEW_LOG_DEBUG("HomePageController::init");
}

void HomePageController::showTestMessage()
{
    UIVIEW_LOG_DEBUG("HomePageController::showTestMessage");
    auto ctx = getAppContext();
    if (!ctx)
    {
        UIVIEW_LOG_WARN("no AppContext");
        return;
    }

    UIView::UIViewMessageBoxHelper::showOkCancel(*ctx,
        QObject::tr("Test Message"),
        QObject::tr("This is a UIViewMessageBoxHelper::showOkCancel test."),
        [](bool accepted) {
            UIVIEW_LOG_DEBUG("test message closed, accepted=" << accepted);
        },
        QObject::tr("Click any button to dismiss the dialog."));
}

void HomePageController::openCamera()
{
    UIVIEW_LOG_DEBUG("HomePageController::openCamera");
    auto ctx = getAppContext();
    if (!ctx)
    {
        UIVIEW_LOG_WARN("no AppContext");
        return;
    }

    auto win = getViewFactory()->createQmlItemWindow(
        kMediaCameraViewQml);
    if (!win)
    {
        UIVIEW_LOG_WARN("failed to create MediaCameraView window");
        return;
    }
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

void HomePageController::openCameraMonitor()
{
    UIVIEW_LOG_DEBUG("HomePageController::openCameraMonitor");
    auto ctx = getAppContext();
    if (!ctx)
    {
        UIVIEW_LOG_WARN("no AppContext");
        return;
    }

    auto win = getViewFactory()->createQmlWindow(
        kCameraMonitorViewQml);
    if (!win)
    {
        UIVIEW_LOG_WARN("failed to create CameraMonitorView window");
        return;
    }
    if (auto* monitorController = UIUtilities::QmlWindowPropertyResolver::resolveObjectAs<CameraMonitorViewController>(
            win, "controller"))
    {
        setupController(monitorController);
    }
    UIUtilities::WindowGeometry::centerOnParentWhenShown(win);
    win->show();
}
