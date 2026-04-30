#pragma once

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>
#include <QtQuick/QQuickWindow>
#include <functional>

#include "UIViewCommon/UIViewHelper/include/UIMessageOptions.h"

class QWindow;
class QScreen;
class AppContext;

namespace UIAppCore {
class UIController;
}

namespace UIView {

// Static utilities for UI concerns that cut across views: window geometry,
// controller extraction from a QML window, and a generic asynchronous
// message dialog.
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

    // ---------- Controller extraction ----------

    // Reads the QML `controller` property from `window` and qobject_casts to T.
    // Returns nullptr on any mismatch.
    //
    // Resolution order:
    //   1. window->property("controller")  — for Window-as-root QMLs created
    //      via IUIViewFactory::createQmlWindow().
    //   2. If `window` is a QQuickView, fall back to its rootObject()'s
    //      `controller` property — for Item-as-root QMLs created via
    //      IUIViewFactory::createQmlItemWindow(). In that case `controller`
    //      should be declared as a property of the root Item.
    template <typename T = UIAppCore::UIController>
    static T* controllerOf(QQuickWindow* window);

    // ---------- Generic Message Dialog ----------

    using MessageCallback = std::function<void(const UIMessageResult&)>;

    // Async = the user's button click is delivered via callback.
    // The QML load itself is synchronous.
    //
    // `appContext` provides the IUIViewFactory used to instantiate the dialog.
    // `onClosed` may be empty if the caller does not need the result.
    static void showMessageAsync(AppContext& appContext,
                                 const UIMessageOptions& opts,
                                 MessageCallback onClosed = {});
};

template <typename T>
T* UIViewHelper::controllerOf(QQuickWindow* window)
{
    if (!window)
    {
        return nullptr;
    }
    QVariant v = window->property("controller");
    if (!v.isValid())
    {
        // Item-as-root path: controller lives on the root Item, not on the
        // QQuickView container.
        if (auto* view = qobject_cast<QQuickView*>(window))
        {
            if (auto* root = view->rootObject())
            {
                v = root->property("controller");
            }
        }
    }
    if (!v.isValid())
    {
        return nullptr;
    }
    return qobject_cast<T*>(v.value<QObject*>());
}

} // namespace UIView
