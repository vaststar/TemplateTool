#pragma once

#include <QtCore/QRect>

class QWindow;
class QScreen;

namespace UIView {

// Static utilities for window geometry shared across views.
//
// For message dialogs, see UIViewMessageBoxHelper.
//
// All methods must be invoked on the UI thread.
class UIViewHelper
{
public:
    // ---------- Window geometry ----------

    // Center `window` over `parent` immediately. Requires the window's size
    // to be already resolved. If `parent` is null, attempts to find a visible
    // top-level window other than `window`; if none, does nothing.
    static void centerOnParent(QWindow* window, QWindow* parent = nullptr);

    // One-shot centering on the first `visibleChanged(true)`. Solves the case
    // where QML width/height bindings are not resolved yet at creation time,
    // and where some window managers ignore setPosition() on not-yet-mapped
    // windows. Auto-disconnects after firing.
    //
    // If no parent can be found (explicit or auto), this is a no-op.
    static void centerOnParentWhenShown(QWindow* window, QWindow* parent = nullptr);

    // Center `window` on `screen`, the window's current screen, or the
    // primary screen, in that order of preference.
    static void centerOnScreen(QWindow* window, QScreen* screen = nullptr);

    // Pure: returns `windowRect` adjusted to fit inside the screen's available
    // area (shrink, then position-preserving push-back). Callable from QML.
    static QRect fitRect(const QRect& windowRect, QScreen* screen = nullptr);

    // Applies fitRect() to `window` in place.
    static void clampIntoScreen(QWindow* window, QScreen* screen = nullptr);

    // ---------- Low-level helpers ----------
    // Exposed so other helpers (e.g. UIViewMessageBoxHelper) can reuse the
    // same fallback-parent / centering primitives without duplicating logic.

    // Picks any visible top-level window other than `self`. Returns nullptr
    // if none exists. Useful as a fallback transient parent when the caller
    // did not provide one.
    static QWindow* findFallbackParent(QWindow* self);

    // Positions `window` at the centre of `reference` (in screen coords).
    // Requires `window`'s size to be already resolved.
    static void positionCenter(QWindow* window, const QRect& reference);
};

} // namespace UIView
