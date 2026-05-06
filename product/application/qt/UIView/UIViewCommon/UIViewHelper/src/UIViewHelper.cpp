#include "UIViewHelper/UIViewHelper.h"

#include <QGuiApplication>
#include <QPointer>
#include <QScreen>
#include <QTimer>
#include <QWindow>

#include <memory>

namespace UIView {

QWindow* UIViewHelper::findFallbackParent(QWindow* self)
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

void UIViewHelper::positionCenter(QWindow* window, const QRect& reference)
{
    const int x = reference.x() + (reference.width()  - window->width())  / 2;
    const int y = reference.y() + (reference.height() - window->height()) / 2;
    window->setPosition(x, y);
}

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

} // namespace UIView
