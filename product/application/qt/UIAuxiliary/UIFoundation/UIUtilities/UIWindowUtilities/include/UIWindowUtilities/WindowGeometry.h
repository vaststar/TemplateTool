#pragma once

#include <QtCore/QRect>

#include <UIWindowUtilities/UIWindowUtilitiesExport.h>

class QScreen;
class QWindow;

namespace UIUtilities {

// Window positioning and screen-boundary operations shared by Qt views.
// All methods must be invoked on the UI thread.
class UIWindowUtilities_EXPORT WindowGeometry final
{
public:
    static void centerOnParent(QWindow* window, QWindow* parent = nullptr);
    static void centerOnParentWhenShown(QWindow* window, QWindow* parent = nullptr);
    static void centerOnScreen(QWindow* window, QScreen* screen = nullptr);

    static QRect fitRect(const QRect& windowRect, QScreen* screen = nullptr);
    static void clampIntoScreen(QWindow* window, QScreen* screen = nullptr);

    static QWindow* findFallbackParent(QWindow* self);
    static void positionCenter(QWindow* window, const QRect& reference);
};

} // namespace UIUtilities
