#include "HomePage/HomePageController.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
#include "UIViewHelper/UIViewHelper.h"
#include <UTMessageDialog/UTMessageOptions.h>
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

    UTMessageOptions opts;
    opts.title   = QObject::tr("Test Message");
    opts.message = QObject::tr("This is a UIViewHelper::showMessageAsync test.");
    opts.detail  = QObject::tr("Click any button to dismiss the dialog.");
    opts.icon    = UTMessageIcon::Info;
    opts.buttons = {
        UTMessageButton{ QObject::tr("OK"),     {}, UTButtonRole::Accept, /*isDefault*/ true,  /*isCancel*/ false },
        UTMessageButton{ QObject::tr("Cancel"), {}, UTButtonRole::Reject, /*isDefault*/ false, /*isCancel*/ true  },
    };

    UIView::UIViewHelper::showMessageAsync(*ctx, opts,
        [](const UTMessageResult& r) {
            UIVIEW_LOG_DEBUG("test message closed, buttonIndex=" << r.buttonIndex
                             << ", buttonText=" << r.buttonText.toStdString());
        });
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
