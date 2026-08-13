#include <UIViewMessageBox/UIViewMessageBoxHelper.h>

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIQmlUtilities/QmlWindowPropertyResolver.h>
#include <UIWindowUtilities/WindowGeometry.h>
#include <UTMessageDialog/UTMessageDialogController.h>
#include <UTMessageDialog/UTMessagePresets.h>

#include <QtCore/QPointer>
#include <QtQuick/QQuickWindow>
#include <QtGui/QWindow>

#include "LoggerDefine/LoggerDefine.h"

namespace {
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

    QPointer<QQuickWindow> window = factory->createQmlWindow(kUTMessageDialogQml);
    if (!window)
    {
        UIVIEW_LOG_WARN("UIViewMessageBoxHelper::showMessageAsync: failed to create UTMessageDialog");
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    auto* controller = UIUtilities::QmlWindowPropertyResolver::resolveObjectAs<UTMessageDialogController>(
        window.data(), "controller");
    if (!controller)
    {
        UIVIEW_LOG_WARN("UIViewMessageBoxHelper::showMessageAsync: UTMessageDialog has no UTMessageDialogController");
        window->deleteLater();
        if (onClosed) onClosed({ -1, {} });
        return;
    }

    controller->setOptions(opts);
    if (onClosed)
    {
        QObject::connect(controller, &UTMessageDialogController::closed,
                         controller,
                         [onClosed](const UTMessageResult& result) { onClosed(result); });
    }

    QWindow* parentWindow = opts.parent
        ? opts.parent
        : UIUtilities::WindowGeometry::findFallbackParent(window.data());
    if (parentWindow)
    {
        window->setTransientParent(parentWindow);
    }

    UIUtilities::WindowGeometry::centerOnParentWhenShown(window.data(), opts.parent);
    window->show();
}

namespace {

UIViewMessageBoxHelper::MessageCallback toBoolAdapter(UIViewMessageBoxHelper::BoolCallback callback,
                                                      UTButtonRole acceptedRole)
{
    if (!callback) return {};
    return [callback = std::move(callback), acceptedRole](const UTMessageResult& result) {
        callback(result.buttonIndex >= 0 && result.role == acceptedRole);
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
