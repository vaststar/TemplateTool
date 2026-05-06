#include "HomePage/HomePageController.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
#include "UIViewHelper/UIViewHelper.h"
#include "UIViewHelper/UIViewMessageBoxHelper.h"
#include "MediaCameraView/MediaCameraViewController.h"

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>

HomePageController::HomePageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create HomePageController");
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

    auto win = ctx->getViewFactory()->createQmlWindow(
        QStringLiteral("UIView/MediaCameraView/qml/MediaCameraView.qml"));
    if (!win)
    {
        UIVIEW_LOG_WARN("failed to create MediaCameraView window");
        return;
    }
    if (auto* mediaController = UIView::UIViewHelper::controllerOf<MediaCameraViewController>(win))
    {
        mediaController->initializeController(ctx.data());
    }
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
}
