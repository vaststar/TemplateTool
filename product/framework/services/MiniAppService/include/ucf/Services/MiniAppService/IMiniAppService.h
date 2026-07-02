#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/MiniAppService/MiniAppManifest.h>
#include <ucf/Services/MiniAppService/IMiniAppServiceCallback.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

/// Framework service that owns the mini-app domain: discovering installed
/// packages, parsing/validating their manifests, installing/uninstalling
/// packages, and exposing each app's isolated package/storage/cache
/// directories. It does NOT render anything — the UI/rendering layer consumes
/// this service for metadata and paths.
class SERVICE_EXPORT IMiniAppService
    : public IService
    , public virtual ucf::utilities::INotificationHelper<IMiniAppServiceCallback>
{
public:
    IMiniAppService() = default;
    IMiniAppService(const IMiniAppService&) = delete;
    IMiniAppService(IMiniAppService&&) = delete;
    IMiniAppService& operator=(const IMiniAppService&) = delete;
    IMiniAppService& operator=(IMiniAppService&&) = delete;
    virtual ~IMiniAppService() = default;

public:
    /// Whether the initial package scan has completed. Consumers can read
    /// listInstalledApps() directly once true; otherwise they should subscribe
    /// and wait for IMiniAppServiceCallback::onMiniAppServiceReady().
    [[nodiscard]] virtual bool isReady() const = 0;

    /// Return the manifests of all currently installed mini-apps.
    /// Result reflects the last scan; invalid packages are skipped.
    [[nodiscard]] virtual std::vector<model::MiniAppManifest> listInstalledApps() const = 0;

    /// Look up a single installed app by id. Returns nullopt if not installed.
    [[nodiscard]] virtual std::optional<model::MiniAppManifest> getApp(const std::string& id) const = 0;

    /// Install a mini-app by copying a source directory (which must contain a
    /// valid manifest.json) into <packages>/<id>/. Returns true on success.
    /// Fails if the manifest is invalid or the id is already installed.
    virtual bool installFromDirectory(const std::string& sourceDirectory) = 0;

    /// Remove an installed app: deletes its package, storage and cache dirs.
    /// Returns true if the app was installed and removed.
    virtual bool uninstall(const std::string& id) = 0;

    /// Absolute path of the app's read-only package directory
    /// (<packages>/<id>). Empty string if id is invalid.
    [[nodiscard]] virtual std::string getAppPackageDir(const std::string& id) const = 0;

    /// Absolute path of the app's private, persistent storage directory
    /// (<storage>/<id>). Lazily created. Empty string if id is invalid.
    [[nodiscard]] virtual std::string getAppStorageDir(const std::string& id) const = 0;

    /// Absolute path of the app's purgeable cache directory (<cache>/<id>).
    /// Lazily created. Empty string if id is invalid.
    [[nodiscard]] virtual std::string getAppCacheDir(const std::string& id) const = 0;

    /// Absolute path of the app's icon file, resolved from the manifest's
    /// (package-relative) icon field. Returns an empty string if the app has no
    /// icon, the icon field is unsafe (absolute or escapes the package dir), or
    /// the file does not exist. Callers should fall back to a placeholder.
    [[nodiscard]] virtual std::string getAppIconPath(const std::string& id) const = 0;

public:
    static std::shared_ptr<IMiniAppService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};

} // namespace ucf::service
