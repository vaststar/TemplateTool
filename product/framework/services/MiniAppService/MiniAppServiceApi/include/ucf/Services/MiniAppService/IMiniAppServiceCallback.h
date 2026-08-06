#pragma once

#include <string>

#include <ucf/Services/MiniAppService/MiniAppManifest.h>

namespace ucf::service {

/// Why an install attempt failed.
enum class MiniAppInstallError
{
    SourceNotFound,      ///< Source directory is missing or not a directory.
    InvalidManifest,     ///< manifest.json is missing/invalid or its id mismatches.
    AlreadyInstalled,    ///< An app with the same id is already installed.
    StorageUnavailable,  ///< Packages root is unavailable or cannot be created.
    CopyFailed,          ///< Copying the package files failed.
    Unknown              ///< Unspecified failure.
};

/// Why an uninstall attempt failed.
enum class MiniAppUninstallError
{
    NotInstalled,        ///< No installed app matches the given id.
    RemoveFailed,        ///< The package directory could not be removed.
    Unknown              ///< Unspecified failure.
};

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

    /// An install attempt failed. Carries the reason.
    virtual void onMiniAppInstallFailed(MiniAppInstallError /*error*/) {}

    /// A mini-app was uninstalled. Carries the removed app's id.
    virtual void onMiniAppUninstalled(const std::string& /*id*/) {}

    /// An uninstall attempt failed. Carries the reason.
    virtual void onMiniAppUninstallFailed(MiniAppUninstallError /*error*/) {}
};

} // namespace ucf::service
