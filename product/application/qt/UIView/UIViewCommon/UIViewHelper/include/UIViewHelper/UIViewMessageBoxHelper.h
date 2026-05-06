#pragma once

#include <QtCore/QString>
#include <functional>

#include <UTMessageDialog/UTMessageOptions.h>

class QWindow;
class AppContext;

namespace UIView {

// Static utilities for showing message dialogs (UTMessageDialog) without
// having to plumb the QML factory + controller wiring at every call site.
//
// All methods must be invoked on the UI thread.
//
// See UIViewHelper for unrelated UI utilities (window centering, controller
// extraction).
class UIViewMessageBoxHelper
{
public:
    // ---------- Generic Message Dialog ----------

    using MessageCallback = std::function<void(const UTMessageResult&)>;

    // Async = the user's button click is delivered via callback.
    // The QML load itself is synchronous.
    //
    // Lifetime: the dialog window is auto-deleted on close (installCloseHandler
    // in UIViewFactory). The QML-owned UTMessageDialogController dies with it.
    // The C++ side never owns either object.
    //
    // `appContext` provides the IUIViewFactory used to instantiate the dialog.
    // `onClosed` may be empty if the caller does not need the result.
    static void showMessageAsync(AppContext& appContext,
                                 const UTMessageOptions& opts,
                                 MessageCallback onClosed = {});

    // ---------- Convenience message dialogs ----------
    //
    // Thin wrappers around showMessageAsync + UTMessagePresets. Use these for
    // the 90% case where you don't need to customise the button list. For
    // anything more involved (3+ buttons, custom roles/tooltips), build a
    // UTMessageOptions yourself (optionally starting from a UTMessagePresets
    // factory) and call showMessageAsync directly.

    // Single-button [OK] notifications. Fire-and-forget.
    static void showInfo   (AppContext& appContext, QString title, QString message,
                            QString detail = {}, QWindow* parent = nullptr);
    static void showWarning(AppContext& appContext, QString title, QString message,
                            QString detail = {}, QWindow* parent = nullptr);
    static void showError  (AppContext& appContext, QString title, QString message,
                            QString detail = {}, QWindow* parent = nullptr);
    static void showSuccess(AppContext& appContext, QString title, QString message,
                            QString detail = {}, QWindow* parent = nullptr);

    // bool == true  -> user picked the Accept-role button (Yes / OK / destructive)
    // bool == false -> user picked Reject / Cancel / closed the window
    using BoolCallback = std::function<void(bool accepted)>;

    // Two-button [Yes][No] confirmation. Default = Yes, Esc = No.
    static void showConfirm(AppContext& appContext, QString title, QString message,
                            BoolCallback onAnswered,
                            QString detail = {}, QWindow* parent = nullptr);

    // Two-button [OK][Cancel]. Default = OK, Esc = Cancel.
    static void showOkCancel(AppContext& appContext, QString title, QString message,
                             BoolCallback onAnswered,
                             QString detail = {}, QWindow* parent = nullptr);

    // Destructive confirmation. Layout: [Cancel] [<destructiveText>], Warning
    // icon, destructive button is NOT the default. `accepted == true` means
    // the user clicked the destructive button.
    static void showDestructiveConfirm(AppContext& appContext, QString title, QString message,
                                       QString destructiveText,
                                       BoolCallback onAnswered,
                                       QString detail = {}, QWindow* parent = nullptr);
};

} // namespace UIView
