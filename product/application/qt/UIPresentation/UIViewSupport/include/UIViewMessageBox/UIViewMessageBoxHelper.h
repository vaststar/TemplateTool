#pragma once

#include <functional>

#include <QtCore/QString>

#include <UTMessageDialog/UTMessageOptions.h>
#include <UIViewSupport/UIViewSupportExport.h>

class AppContext;
class QWindow;

namespace UIView {

// Application-level adapter for presenting UTMessageDialog windows through
// AppContext. All methods must be invoked on the UI thread.
class UIViewSupport_EXPORT UIViewMessageBoxHelper
{
public:
    using MessageCallback = std::function<void(const UTMessageResult&)>;

    static void showMessageAsync(AppContext& appContext,
                                 const UTMessageOptions& opts,
                                 MessageCallback onClosed = {});

    static void showInfo(AppContext& appContext, QString title, QString message,
                         QString detail = {}, QWindow* parent = nullptr);
    static void showWarning(AppContext& appContext, QString title, QString message,
                            QString detail = {}, QWindow* parent = nullptr);
    static void showError(AppContext& appContext, QString title, QString message,
                          QString detail = {}, QWindow* parent = nullptr);
    static void showSuccess(AppContext& appContext, QString title, QString message,
                            QString detail = {}, QWindow* parent = nullptr);

    using BoolCallback = std::function<void(bool accepted)>;

    static void showConfirm(AppContext& appContext, QString title, QString message,
                            BoolCallback onAnswered,
                            QString detail = {}, QWindow* parent = nullptr);
    static void showOkCancel(AppContext& appContext, QString title, QString message,
                             BoolCallback onAnswered,
                             QString detail = {}, QWindow* parent = nullptr);
    static void showDestructiveConfirm(AppContext& appContext, QString title, QString message,
                                       QString destructiveText,
                                       BoolCallback onAnswered,
                                       QString detail = {}, QWindow* parent = nullptr);
};

} // namespace UIView
