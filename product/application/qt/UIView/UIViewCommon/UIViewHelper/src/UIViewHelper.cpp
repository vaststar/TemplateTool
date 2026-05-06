#include "UIViewHelper/UIViewHelper.h"

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UTMessageDialog/UTMessageDialogController.h>

#include <QGuiApplication>
#include <QPointer>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>
#include <QWindow>

#include <memory>

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace UIView {

namespace {

// Picks any visible top-level window other than `self`. Used as a fallback
// parent when the caller did not provide one.
QWindow* findFallbackParent(QWindow* self)
{
    for (QWindow* tlw : QGuiApplication::topLevelWindows())
    {
        if (tlw != self && tlw->isVisible())
        {
            return tlw;
        }
    }
    return nullptr;
}

void positionCenter(QWindow* window, const QRect& reference)
{
    const int x = reference.x() + (reference.width()  - window->width())  / 2;
    const int y = reference.y() + (reference.height() - window->height()) / 2;
    window->setPosition(x, y);
}

} // namespace

void UIViewHelper::centerOnParent(QWindow* window, QWindow* parent)
{
    if (!window)
    {
        return;
    }
    QWindow* p = parent ? parent : findFallbackParent(window);
    if (!p)
    {
        return;
    }
    positionCenter(window, p->geometry());
}

void UIViewHelper::centerOnParentWhenShown(QWindow* window, QWindow* parent)
{
    if (!window)
    {
        return;
    }
    QWindow* p = parent ? parent : findFallbackParent(window);
    if (!p)
    {
        // No parent available (e.g. the very first window). Skip silently;
        // callers that want screen-centering should call centerOnScreen.
        return;
    }

    // Two-phase centering:
    //   (1) Center immediately. If the QML width/height bindings are already
    //       resolved (typical for static-sized dialogs declaring `width: 400`),
    //       this avoids any visible jump.
    //   (2) Re-center on the first visibleChanged(true), deferred via
    //       QTimer::singleShot(0, ...). On Windows/Qt the platform may apply
    //       a default placement during show() that overrides a setPosition()
    //       called from inside the visibleChanged emission. Deferring to the
    //       next event-loop iteration ensures the platform finished its show
    //       pipeline before we move the window. Auto-disconnects after firing.
    positionCenter(window, p->geometry());

    auto centerOnce = std::make_shared<QMetaObject::Connection>();
    QPointer<QWindow> parentGuard(p);
    QPointer<QWindow> windowGuard(window);

    *centerOnce = QObject::connect(window, &QWindow::visibleChanged,
        [windowGuard, parentGuard, centerOnce](bool visible)
    {
        if (!visible)
        {
            return;
        }
        QObject::disconnect(*centerOnce);
        QTimer::singleShot(0, [windowGuard, parentGuard]()
        {
            if (!windowGuard || !parentGuard)
            {
                return;
            }
            positionCenter(windowGuard.data(), parentGuard->geometry());
        });
    });
}

void UIViewHelper::centerOnScreen(QWindow* window, QScreen* screen)
{
    if (!window)
    {
        return;
    }
    QScreen* s = screen ? screen : window->screen();
    if (!s)
    {
        s = QGuiApplication::primaryScreen();
    }
    if (!s)
    {
        return;
    }
    positionCenter(window, s->availableGeometry());
}

void UIViewHelper::showMessageAsync(AppContext& appContext,
                                    const UTMessageOptions& opts,
                                    MessageCallback onClosed)
{
    if (opts.buttons.isEmpty())
    {
        UIVIEW_LOG_WARN("UIViewHelper::showMessageAsync: no buttons; aborting");
        Q_ASSERT_X(false, "UIViewHelper::showMessageAsync", "buttons must not be empty");
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    auto factory = appContext.getViewFactory();
    if (!factory)
    {
        UIVIEW_LOG_WARN("UIViewHelper::showMessageAsync: no view factory");
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    // The dialog's QML root creates its own UTMessageDialogController, so we
    // do NOT pass any initialProperties. The window owns the controller via
    // the QML object tree; both die together when the window closes.
    QPointer<QQuickWindow> win = factory->createQmlWindow(
        QStringLiteral("UTComposite/UTMessageDialog/UTMessageDialog.qml"));
    if (!win)
    {
        UIVIEW_LOG_WARN("UIViewHelper::showMessageAsync: failed to create UTMessageDialog");
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    auto* controller = controllerOf<UTMessageDialogController>(win.data());
    if (!controller)
    {
        UIVIEW_LOG_WARN("UIViewHelper::showMessageAsync: UTMessageDialog has no UTMessageDialogController");
        win->deleteLater();
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    controller->setOptions(opts);
    if (onClosed)
    {
        // `controller` is the connection's context: when it is destroyed
        // (together with `win`), the connection is removed automatically.
        QObject::connect(controller, &UTMessageDialogController::closed,
                         controller,
                         [onClosed](const UTMessageResult& r) { onClosed(r); });
    }

    // Mark the dialog as owned by `parent` so that:
    //   * Windows: no separate taskbar entry; the dialog inherits the parent's
    //     window icon (avoids the "blank icon" issue).
    //   * macOS / Linux: appropriate stacking and modality semantics.
    // Falls back to any visible top-level window when no explicit parent is
    // given, mirroring centerOnParentWhenShown()'s behaviour.
    QWindow* parentWindow = opts.parent
        ? opts.parent
        : findFallbackParent(win.data());
    if (parentWindow)
    {
        win->setTransientParent(parentWindow);
    }

    centerOnParentWhenShown(win.data(), opts.parent);
    win->show();
}

} // namespace UIView
