#pragma once

#include <cstdint>
#include <memory>

#include <QString>

#include "UIViewBase/UIViewController.h"

namespace commonHead::viewModels
{
    class IMiniAppRuntimeViewModel;
}

namespace UIVMSignalEmitter
{
    class MiniAppRuntimeViewModelEmitter;
}

namespace MiniAppsPage {

/**
 * @brief Controller for a single running mini-app, owned by its host window.
 *
 * A UIViewController the host window sets up via setupController(): on init() it
 * creates its own runtime view model, starts the runtime, and exposes the load
 * lifecycle as Qt signals. It knows nothing about the window — the window owns
 * and observes it, mirroring how a QML view binds to its controller.
 */
class MiniAppInstanceController : public UIViewController
{
    Q_OBJECT
public:
    explicit MiniAppInstanceController(QString appId, QObject* parent = nullptr);
    ~MiniAppInstanceController() override;

    const QString& appId() const { return mAppId; }

    // Native web-view window handle to embed, or 0 if the runtime has not
    // started or exposes no embeddable backend. Valid after runtimeStarted().
    std::uintptr_t nativeHostHandle() const;

signals:
    // Emitted once the runtime has started and nativeHostHandle() is available.
    void runtimeStarted();
    void loadFinished(bool ok);
    void loadFailed(int code, const QString& message);

protected:
    // Creates the runtime view model and starts the runtime.
    void init() override;

private:
    std::shared_ptr<commonHead::viewModels::IMiniAppRuntimeViewModel> mViewModel;
    std::shared_ptr<UIVMSignalEmitter::MiniAppRuntimeViewModelEmitter> mEmitter;
    QString mAppId;
};

} // namespace MiniAppsPage
