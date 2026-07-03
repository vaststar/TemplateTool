#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <ucf/Utilities/SinkNotifier/SinkNotifier.h>
#include <ucf/Services/MiniAppService/MiniAppManifest.h>

#include "MiniAppNotificationSink.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

/// Owns all mini-app filesystem/domain logic. Keeps an in-memory snapshot of
/// installed apps (rebuilt by rescan()) and resolves per-app isolated
/// directories via IClientInfoService path roots. Fires state-change events
/// upward to the MiniAppService via the SinkNotifier mechanism.
class MiniAppManager final : public ucf::utilities::SinkNotifier<IMiniAppNotificationSink>
{
public:
    explicit MiniAppManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~MiniAppManager();
    MiniAppManager(const MiniAppManager&) = delete;
    MiniAppManager(MiniAppManager&&) = delete;
    MiniAppManager& operator=(const MiniAppManager&) = delete;
    MiniAppManager& operator=(MiniAppManager&&) = delete;

public:
    /// Scan the packages root from disk to build the initial in-memory
    /// snapshot, then notify the sink that the service is ready.
    void rescan();

    /// True once the first rescan() has completed.
    [[nodiscard]] bool isReady() const;

    [[nodiscard]] std::vector<model::MiniAppManifest> listInstalledApps() const;
    [[nodiscard]] std::optional<model::MiniAppManifest> getApp(const std::string& id) const;

    /// Install a package from a source directory. On success adds it to the
    /// in-memory snapshot and fires onMiniAppInstalled; on failure fires
    /// onMiniAppInstallFailed with the reason.
    void installFromDirectory(const std::string& sourceDirectory);

    /// Uninstall an app: removes its dirs and drops it from the in-memory
    /// snapshot. On success fires onMiniAppUninstalled; on failure fires
    /// onMiniAppUninstallFailed with the reason.
    void uninstall(const std::string& id);

    [[nodiscard]] std::string getAppPackageDir(const std::string& id) const;
    [[nodiscard]] std::string getAppStorageDir(const std::string& id) const;
    [[nodiscard]] std::string getAppCacheDir(const std::string& id) const;

    /// Resolve the app's icon file to an absolute path from the manifest's
    /// package-relative icon field. Returns "" if there is no icon, the icon
    /// field is unsafe (absolute or contains ".."), or the file is missing.
    [[nodiscard]] std::string getAppIconPath(const std::string& id) const;

private:
    [[nodiscard]] std::string packagesRoot() const;
    [[nodiscard]] std::string storageRoot() const;
    [[nodiscard]] std::string cacheRoot() const;

    /// Read + parse + validate a single package directory's manifest.json.
    /// Returns nullopt (with a logged warning) if missing or invalid.
    [[nodiscard]] std::optional<model::MiniAppManifest> loadManifest(
        const std::string& packageDir, const std::string& expectedId) const;

private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    mutable std::mutex mMutex;
    std::vector<model::MiniAppManifest> mInstalledApps;
    std::atomic<bool> mReady{false};
};

} // namespace ucf::service
