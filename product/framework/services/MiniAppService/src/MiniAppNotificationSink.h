#pragma once

#include <string>

#include <ucf/Services/MiniAppService/MiniAppManifest.h>

namespace ucf::service {

/// Internal sink: the MiniAppManager invokes it after a state change; the
/// MiniAppService implements it and translates each event into an outward
/// IMiniAppServiceCallback notification. This mirrors the SinkNotifier /
/// NotificationHelper split used by other services (e.g. ContactService).
class IMiniAppNotificationSink
{
public:
    virtual ~IMiniAppNotificationSink() = default;

    /// The initial scan of installed packages finished.
    virtual void onMiniAppServiceReady() = 0;

    /// A mini-app was successfully installed; carries its manifest.
    virtual void onMiniAppInstalled(const model::MiniAppManifest& app) = 0;

    /// A mini-app was uninstalled; carries its id.
    virtual void onMiniAppUninstalled(const std::string& id) = 0;
};

} // namespace ucf::service
