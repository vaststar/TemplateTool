#pragma once

#include <QObject>
#include <QString>

#include <memory>

namespace commonHead::viewModels {
    class IMiniAppRuntimeViewModel;
}

namespace MiniAppRuntime {

// Public entry point of the mini-app runtime and the only class the rest of the
// app touches. Keeps one live window per app id; internals are hidden via Pimpl.
class MiniAppRuntimeManager : public QObject
{
    Q_OBJECT
public:
    explicit MiniAppRuntimeManager(QObject* parent = nullptr);
    ~MiniAppRuntimeManager() override; // defined in .cpp (Pimpl)

    // Open the app identified by `appId`, driving it through the given runtime
    // view model, or raise its window (ignoring `viewModel`) if already open.
    void launch(const std::shared_ptr<commonHead::viewModels::IMiniAppRuntimeViewModel>& viewModel,
                const QString& appId,
                const QString& displayName);

    // Close a running app's window, if any.
    void close(const QString& appId);

    [[nodiscard]] bool isRunning(const QString& appId) const;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace MiniAppRuntime
