#pragma once

#include <string>

#include <ucf/Services/MiniAppService/MiniAppManifest.h>

namespace ucf::service {

/// Sink interface for mini-app lifecycle events. All methods have default
/// (empty) implementations so subscribers only override what they care about.
class IMiniAppServiceCallback
{
public:
    IMiniAppServiceCallback() = default;
    IMiniAppServiceCallback(const IMiniAppServiceCallback&) = delete;
    IMiniAppServiceCallback(IMiniAppServiceCallback&&) = delete;
    IMiniAppServiceCallback& operator=(const IMiniAppServiceCallback&) = delete;
    IMiniAppServiceCallback& operator=(IMiniAppServiceCallback&&) = delete;
    virtual ~IMiniAppServiceCallback() = default;

public:
    /// The initial scan of installed packages finished; listInstalledApps() is
    /// now populated and safe to read.
    virtual void onMiniAppServiceReady() {}

    /// A mini-app was successfully installed. Carries the newly installed
    /// app's manifest.
    virtual void onMiniAppInstalled(const model::MiniAppManifest& /*app*/) {}

    /// A mini-app was uninstalled. Carries the removed app's id.
    virtual void onMiniAppUninstalled(const std::string& /*id*/) {}
};

} // namespace ucf::service
