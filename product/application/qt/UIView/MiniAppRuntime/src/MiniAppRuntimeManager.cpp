#include "MiniAppRuntime/MiniAppRuntimeManager.h"

#include <QHash>
#include <QPointer>

#include "MiniAppHostWindow.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace MiniAppRuntime {

struct MiniAppRuntimeManager::Impl
{
    // One window per app id. QPointer auto-clears when a window self-destroys.
    QHash<QString, QPointer<MiniAppHostWindow>> windows;
};

MiniAppRuntimeManager::MiniAppRuntimeManager(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Impl>())
{
}

MiniAppRuntimeManager::~MiniAppRuntimeManager() = default;

void MiniAppRuntimeManager::launch(const std::shared_ptr<commonHead::viewModels::IMiniAppRuntimeViewModel>& viewModel,
                                   const QString& appId,
                                   const QString& displayName)
{
    if (appId.isEmpty())
    {
        UIVIEW_LOG_WARN("MiniAppRuntimeManager::launch ignored empty app id");
        return;
    }

    if (auto* existing = d->windows.value(appId).data())
    {
        UIVIEW_LOG_INFO("MiniAppRuntimeManager::launch raising existing window for "
                        << appId.toStdString());
        existing->show();
        existing->raise();
        existing->activateWindow();
        return;
    }

    if (!viewModel)
    {
        UIVIEW_LOG_WARN("MiniAppRuntimeManager::launch null view model for " << appId.toStdString());
        return;
    }

    auto* window = new MiniAppHostWindow(viewModel, appId, displayName);
    d->windows.insert(appId, window);
    window->show();
}

void MiniAppRuntimeManager::close(const QString& appId)
{
    if (auto* window = d->windows.take(appId).data())
    {
        window->close(); // WA_DeleteOnClose frees it
    }
}

bool MiniAppRuntimeManager::isRunning(const QString& appId) const
{
    return !d->windows.value(appId).isNull();
}

} // namespace MiniAppRuntime
