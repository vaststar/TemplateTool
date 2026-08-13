#pragma once

#include <memory>

#include <QPointer>
#include <QString>
#include <QWidget>

#include <cstdint>

class QLabel;
class QStackedWidget;
class UIViewController;

namespace MiniAppsPage {

class MiniAppInstanceController;

// Top-level host window for a single mini-app. Following the app's convention,
// the view owns its controller: it creates a MiniAppInstanceController, builds
// its status/content pages, and observes the controller's signals to embed the
// native web-view and switch pages. Self-deletes on close.
class MiniAppHostWindow : public QWidget
{
    Q_OBJECT
public:
    MiniAppHostWindow(const QString& appId, const QString& displayName, QWidget* parent = nullptr);
    ~MiniAppHostWindow() override;

    // Set up the owned controller (inject AppContext + run init()) through the
    // launching parent controller, then show the window. Call once, after the
    // window is tracked.
    void initialize(const QPointer<UIViewController>& parent);

private:
    void onRuntimeStarted();
    void onLoadFinished(bool ok);
    void onLoadFailed(int code, const QString& message);

    void setNativeView(std::uintptr_t handle);
    void showLoading(const QString& text);
    void showContent();
    void showError(const QString& text);

    std::unique_ptr<MiniAppInstanceController> mController;
    QStackedWidget* mStack = nullptr;   // page 0: status, page 1: content
    QLabel* mStatusLabel = nullptr;     // loading/error text on the status page
    bool mHasContent = false;           // whether a content page has been added
};

} // namespace MiniAppsPage
