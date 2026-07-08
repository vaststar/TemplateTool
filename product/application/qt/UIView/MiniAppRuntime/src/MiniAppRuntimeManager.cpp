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

void MiniAppRuntimeManager::launch(const MiniAppContext& context)
{
    if (context.id.isEmpty())
    {
        UIVIEW_LOG_WARN("MiniAppRuntimeManager::launch ignored empty app id");
        return;
    }

    if (auto* existing = d->windows.value(context.id).data())
    {
        UIVIEW_LOG_INFO("MiniAppRuntimeManager::launch raising existing window for "
                        << context.id.toStdString());
        existing->show();
        existing->raise();
        existing->activateWindow();
        return;
    }

    auto* window = new MiniAppHostWindow(context);
    d->windows.insert(context.id, window);
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
