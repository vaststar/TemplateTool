#include <UIWindowUtilities/WindowGeometry.h>

#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/QWindow>

#include <memory>

namespace UIUtilities {

QWindow* WindowGeometry::findFallbackParent(QWindow* self)
{
    for (QWindow* topLevelWindow : QGuiApplication::topLevelWindows())
    {
        if (topLevelWindow != self && topLevelWindow->isVisible())
        {
            return topLevelWindow;
        }
    }
    return nullptr;
}

void WindowGeometry::positionCenter(QWindow* window, const QRect& reference)
{
    if (!window)
    {
        return;
    }

    const int x = reference.x() + (reference.width() - window->width()) / 2;
    const int y = reference.y() + (reference.height() - window->height()) / 2;
    window->setPosition(x, y);
}

void WindowGeometry::centerOnParent(QWindow* window, QWindow* parent)
{
    if (!window)
    {
        return;
    }
    QWindow* resolvedParent = parent ? parent : findFallbackParent(window);
    if (!resolvedParent)
    {
        return;
    }
    positionCenter(window, resolvedParent->geometry());
}

void WindowGeometry::centerOnParentWhenShown(QWindow* window, QWindow* parent)
{
    if (!window)
    {
        return;
    }
    QWindow* resolvedParent = parent ? parent : findFallbackParent(window);
    if (!resolvedParent)
    {
        return;
    }

    positionCenter(window, resolvedParent->geometry());

    auto centerOnce = std::make_shared<QMetaObject::Connection>();
    QPointer<QWindow> parentGuard(resolvedParent);
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

void WindowGeometry::centerOnScreen(QWindow* window, QScreen* screen)
{
    if (!window)
    {
        return;
    }
    QScreen* resolvedScreen = screen ? screen : window->screen();
    if (!resolvedScreen)
    {
        resolvedScreen = QGuiApplication::primaryScreen();
    }
    if (!resolvedScreen)
    {
        return;
    }
    positionCenter(window, resolvedScreen->availableGeometry());
}

QRect WindowGeometry::fitRect(const QRect& windowRect, QScreen* screen)
{
    QScreen* resolvedScreen = screen ? screen : QGuiApplication::screenAt(windowRect.center());
    if (!resolvedScreen)
    {
        resolvedScreen = QGuiApplication::primaryScreen();
    }
    if (!resolvedScreen)
    {
        return windowRect;
    }

    const QRect available = resolvedScreen->availableGeometry();

    const int width = qMin(windowRect.width(), available.width());
    const int height = qMin(windowRect.height(), available.height());

    const int x = qBound(available.left(), windowRect.x(), available.right() - width + 1);
    const int y = qBound(available.top(), windowRect.y(), available.bottom() - height + 1);
    return QRect(x, y, width, height);
}

void WindowGeometry::clampIntoScreen(QWindow* window, QScreen* screen)
{
    if (!window)
    {
        return;
    }
    QScreen* resolvedScreen = screen ? screen : window->screen();
    const QRect fitted = fitRect(window->geometry(), resolvedScreen);
    if (fitted.size() != window->size())
    {
        window->resize(fitted.size());
    }
    if (fitted.topLeft() != window->position())
    {
        window->setPosition(fitted.topLeft());
    }
}

} // namespace UIUtilities
