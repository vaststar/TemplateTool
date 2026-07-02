#pragma once

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

    [[nodiscard]] std::vector<model::MiniAppManifest> listInstalledApps() const;
    [[nodiscard]] std::optional<model::MiniAppManifest> getApp(const std::string& id) const;

    /// Install a package from a source directory. On success, adds it to the
    /// in-memory snapshot, notifies the sink, and returns the installed
    /// manifest; returns nullopt on failure.
    [[nodiscard]] std::optional<model::MiniAppManifest> installFromDirectory(const std::string& sourceDirectory);

    /// Uninstall an app: removes its dirs, drops it from the in-memory
    /// snapshot, notifies the sink. Returns true if it was installed & removed.
    bool uninstall(const std::string& id);

    [[nodiscard]] std::string getAppPackageDir(const std::string& id) const;
    [[nodiscard]] std::string getAppStorageDir(const std::string& id) const;
    [[nodiscard]] std::string getAppCacheDir(const std::string& id) const;

private:
    /// Resolve the packages root (data/mini_app/packages) via IClientInfoService.
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
};

} // namespace ucf::service
