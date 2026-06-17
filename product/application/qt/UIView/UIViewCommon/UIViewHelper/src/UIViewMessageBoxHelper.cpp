#include "UIViewHelper/UIViewMessageBoxHelper.h"
#include "UIViewHelper/UIViewHelper.h"

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UTMessageDialog/UTMessageDialogController.h>
#include <UTMessageDialog/UTMessagePresets.h>

#include <QPointer>
#include <QQuickWindow>
#include <QWindow>

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace {
// QML resource path for the shared message dialog.
const QString kUTMessageDialogQml = QStringLiteral("UTComposite/UTMessageDialog/UTMessageDialog.qml");
}

namespace UIView {

void UIViewMessageBoxHelper::showMessageAsync(AppContext& appContext,
                                              const UTMessageOptions& opts,
                                              MessageCallback onClosed)
{
    if (opts.buttons.isEmpty())
    {
        UIVIEW_LOG_WARN("UIViewMessageBoxHelper::showMessageAsync: no buttons; aborting");
        Q_ASSERT_X(false, "UIViewMessageBoxHelper::showMessageAsync", "buttons must not be empty");
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    auto factory = appContext.getViewFactory();
    if (!factory)
    {
        UIVIEW_LOG_WARN("UIViewMessageBoxHelper::showMessageAsync: no view factory");
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    // The dialog's QML root creates its own UTMessageDialogController, so we
    // do NOT pass any initialProperties. The window owns the controller via
    // the QML object tree; both die together when the window closes.
    QPointer<QQuickWindow> win = factory->createQmlWindow(
        kUTMessageDialogQml);
    if (!win)
    {
        UIVIEW_LOG_WARN("UIViewMessageBoxHelper::showMessageAsync: failed to create UTMessageDialog");
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    auto* controller = UIViewHelper::controllerOf<UTMessageDialogController>(win.data());
    if (!controller)
    {
        UIVIEW_LOG_WARN("UIViewMessageBoxHelper::showMessageAsync: UTMessageDialog has no UTMessageDialogController");
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
        : UIViewHelper::findFallbackParent(win.data());
    if (parentWindow)
    {
        win->setTransientParent(parentWindow);
    }

    UIViewHelper::centerOnParentWhenShown(win.data(), opts.parent);
    win->show();
}

// ---------- Convenience message dialogs ----------

namespace {

// Folds a UTMessageResult down to a bool: Accept-role => true, anything else
// (Reject / Destructive / external close) => false.
//
// Note: destructive buttons carry UTButtonRole::Destructive, NOT Accept, so
// callers of showDestructiveConfirm get a custom adapter below instead.
UIViewMessageBoxHelper::MessageCallback toBoolAdapter(UIViewMessageBoxHelper::BoolCallback cb,
                                                      UTButtonRole acceptedRole)
{
    if (!cb) return {};
    return [cb = std::move(cb), acceptedRole](const UTMessageResult& r) {
        cb(r.buttonIndex >= 0 && r.role == acceptedRole);
    };
}

} // namespace

void UIViewMessageBoxHelper::showInfo(AppContext& appContext, QString title, QString message,
                                      QString detail, QWindow* parent)
{
    showMessageAsync(appContext,
        UTMessagePresets::info(std::move(title), std::move(message),
                               std::move(detail), parent));
}

void UIViewMessageBoxHelper::showWarning(AppContext& appContext, QString title, QString message,
                                         QString detail, QWindow* parent)
{
    showMessageAsync(appContext,
        UTMessagePresets::warning(std::move(title), std::move(message),
                                  std::move(detail), parent));
}

void UIViewMessageBoxHelper::showError(AppContext& appContext, QString title, QString message,
                                       QString detail, QWindow* parent)
{
    showMessageAsync(appContext,
        UTMessagePresets::error(std::move(title), std::move(message),
                                std::move(detail), parent));
}

void UIViewMessageBoxHelper::showSuccess(AppContext& appContext, QString title, QString message,
                                         QString detail, QWindow* parent)
{
    showMessageAsync(appContext,
        UTMessagePresets::success(std::move(title), std::move(message),
                                  std::move(detail), parent));
}

void UIViewMessageBoxHelper::showConfirm(AppContext& appContext, QString title, QString message,
                                         BoolCallback onAnswered,
                                         QString detail, QWindow* parent)
{
    showMessageAsync(appContext,
        UTMessagePresets::confirm(std::move(title), std::move(message),
                                  std::move(detail), parent),
        toBoolAdapter(std::move(onAnswered), UTButtonRole::Accept));
}

void UIViewMessageBoxHelper::showOkCancel(AppContext& appContext, QString title, QString message,
                                          BoolCallback onAnswered,
                                          QString detail, QWindow* parent)
{
    showMessageAsync(appContext,
        UTMessagePresets::okCancel(std::move(title), std::move(message),
                                   std::move(detail), parent),
        toBoolAdapter(std::move(onAnswered), UTButtonRole::Accept));
}

void UIViewMessageBoxHelper::showDestructiveConfirm(AppContext& appContext, QString title, QString message,
                                                    QString destructiveText,
                                                    BoolCallback onAnswered,
                                                    QString detail, QWindow* parent)
{
    showMessageAsync(appContext,
        UTMessagePresets::destructiveConfirm(std::move(title), std::move(message),
                                             std::move(destructiveText),
                                             std::move(detail), parent),
        toBoolAdapter(std::move(onAnswered), UTButtonRole::Destructive));
}

} // namespace UIView
