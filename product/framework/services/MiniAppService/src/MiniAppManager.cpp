#include "MiniAppManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>
#include <ucf/Utilities/JsonUtils/JsonValue.h>

#include "MiniAppServiceLogger.h"

namespace fs = std::filesystem;

namespace ucf::service {

namespace {
constexpr auto kManifestFileName = "manifest.json";

std::string readFileUtf8(const fs::path& file)
{
    std::ifstream in(file, std::ios::binary);
    if (!in)
    {
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}
} // namespace

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start MiniAppManager Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

MiniAppManager::MiniAppManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("Create MiniAppManager, address:" << this);
}

MiniAppManager::~MiniAppManager()
{
    SERVICE_LOG_DEBUG("Delete MiniAppManager, address:" << this);
}

std::string MiniAppManager::packagesRoot() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        if (auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            return clientInfoService->getMiniAppPackageStoragePath();
        }
    }
    SERVICE_LOG_WARN("packagesRoot unavailable: ClientInfoService not ready");
    return {};
}

std::string MiniAppManager::storageRoot() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        if (auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            return clientInfoService->getMiniAppDataStoragePath();
        }
    }
    SERVICE_LOG_WARN("storageRoot unavailable: ClientInfoService not ready");
    return {};
}

std::string MiniAppManager::cacheRoot() const
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        if (auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            return clientInfoService->getMiniAppCacheStoragePath();
        }
    }
    SERVICE_LOG_WARN("cacheRoot unavailable: ClientInfoService not ready");
    return {};
}

std::optional<model::MiniAppManifest> MiniAppManager::loadManifest(
    const std::string& packageDir, const std::string& expectedId) const
{
    const fs::path manifestPath =
        ucf::utilities::FilePathUtils::joinPaths(ucf::utilities::FilePathUtils::pathFromUtf8(packageDir), kManifestFileName);

    std::error_code ec;
    if (!fs::exists(manifestPath, ec) || ec)
    {
        SERVICE_LOG_WARN("Skip package '" << expectedId << "': missing manifest.json");
        return std::nullopt;
    }

    const std::string content = readFileUtf8(manifestPath);
    if (content.empty())
    {
        SERVICE_LOG_WARN("Skip package '" << expectedId << "': manifest.json is empty or unreadable");
        return std::nullopt;
    }

    const auto json = ucf::utilities::JsonValue::parse(content);
    if (!json.isObject())
    {
        SERVICE_LOG_WARN("Skip package '" << expectedId << "': manifest.json is not a JSON object");
        return std::nullopt;
    }

    model::MiniAppManifest manifest;
    manifest.id          = json.get("id").asString().value_or("");
    manifest.name        = json.get("name").asString().value_or("");
    manifest.version     = json.get("version").asString().value_or("");
    manifest.description = json.get("description").asString().value_or("");
    manifest.entry       = json.get("entry").asString().value_or("");
    manifest.icon        = json.get("icon").asString().value_or("");

    const auto permissions = json.get("permissions");
    if (permissions.isArray())
    {
        for (const auto& permission : permissions)
        {
            if (auto value = permission.asString())
            {
                manifest.permissions.push_back(*value);
            }
        }
    }

    // Validation: required fields present, and id must match the directory name
    // to guarantee the appId namespace is the sandbox boundary.
    if (manifest.id.empty() || manifest.name.empty() || manifest.version.empty())
    {
        SERVICE_LOG_WARN("Skip package '" << expectedId << "': missing required field(s) id/name/version");
        return std::nullopt;
    }
    if (manifest.id != expectedId)
    {
        SERVICE_LOG_WARN("Skip package '" << expectedId << "': manifest id '" << manifest.id
                                          << "' does not match directory name");
        return std::nullopt;
    }

    return manifest;
}

void MiniAppManager::rescan()
{
    const std::string root = packagesRoot();

    std::vector<model::MiniAppManifest> scanned;
    if (!root.empty())
    {
        const fs::path rootPath = ucf::utilities::FilePathUtils::pathFromUtf8(root);
        std::error_code ec;
        if (fs::exists(rootPath, ec) && fs::is_directory(rootPath, ec))
        {
            for (const auto& entry : fs::directory_iterator(rootPath, ec))
            {
                if (ec)
                {
                    SERVICE_LOG_WARN("rescan: error iterating packages root: " << ec.message());
                    break;
                }
                if (!entry.is_directory(ec) || ec)
                {
                    continue;
                }
                const std::string id = ucf::utilities::FilePathUtils::utf8FromPath(entry.path().filename());
                const std::string packageDir = ucf::utilities::FilePathUtils::utf8FromPath(entry.path());
                if (auto manifest = loadManifest(packageDir, id))
                {
                    scanned.push_back(std::move(*manifest));
                }
            }
        }
    }

    {
        std::lock_guard lock(mMutex);
        mInstalledApps = std::move(scanned);
        SERVICE_LOG_INFO("rescan: " << mInstalledApps.size() << " installed mini-app(s)");
    }

    // Initial snapshot is built; let the service translate this to a ready event.
    notifySink(&IMiniAppNotificationSink::onMiniAppServiceReady);
}

std::vector<model::MiniAppManifest> MiniAppManager::listInstalledApps() const
{
    std::lock_guard lock(mMutex);
    return mInstalledApps;
}

std::optional<model::MiniAppManifest> MiniAppManager::getApp(const std::string& id) const
{
    std::lock_guard lock(mMutex);
    for (const auto& app : mInstalledApps)
    {
        if (app.id == id)
        {
            return app;
        }
    }
    return std::nullopt;
}

std::optional<model::MiniAppManifest> MiniAppManager::installFromDirectory(const std::string& sourceDirectory)
{
    if (sourceDirectory.empty())
    {
        SERVICE_LOG_ERROR("install failed: empty source directory");
        return std::nullopt;
    }

    const fs::path srcPath = ucf::utilities::FilePathUtils::pathFromUtf8(sourceDirectory);
    std::error_code ec;
    if (!fs::exists(srcPath, ec) || !fs::is_directory(srcPath, ec) || ec)
    {
        SERVICE_LOG_ERROR("install failed: source directory does not exist: " << sourceDirectory);
        return std::nullopt;
    }

    // Read the source manifest to determine the target id (dir name == id).
    const std::string srcId = ucf::utilities::FilePathUtils::utf8FromPath(srcPath.filename());
    const auto manifest = loadManifest(sourceDirectory, srcId);
    if (!manifest)
    {
        SERVICE_LOG_ERROR("install failed: invalid manifest in source directory: " << sourceDirectory);
        return std::nullopt;
    }

    const std::string root = packagesRoot();
    if (root.empty())
    {
        SERVICE_LOG_ERROR("install failed: packages root unavailable");
        return std::nullopt;
    }
    if (!ucf::utilities::FilePathUtils::EnsureDirectoryExists(ucf::utilities::FilePathUtils::pathFromUtf8(root)))
    {
        SERVICE_LOG_ERROR("install failed: cannot create packages root: " << root);
        return std::nullopt;
    }

    const fs::path destPath =
        ucf::utilities::FilePathUtils::joinPaths(ucf::utilities::FilePathUtils::pathFromUtf8(root), manifest->id);
    if (fs::exists(destPath, ec))
    {
        SERVICE_LOG_ERROR("install failed: app already installed: " << manifest->id);
        return std::nullopt;
    }

    fs::copy(srcPath, destPath, fs::copy_options::recursive, ec);
    if (ec)
    {
        SERVICE_LOG_ERROR("install failed: copy error for '" << manifest->id << "': " << ec.message());
        fs::remove_all(destPath, ec);
        return std::nullopt;
    }

    // Update the in-memory snapshot incrementally instead of rescanning.
    {
        std::lock_guard lock(mMutex);
        mInstalledApps.push_back(*manifest);
    }

    SERVICE_LOG_INFO("installed mini-app '" << manifest->id << "'");
    notifySink(&IMiniAppNotificationSink::onMiniAppInstalled, *manifest);
    return manifest;
}

bool MiniAppManager::uninstall(const std::string& id)
{
    if (id.empty())
    {
        SERVICE_LOG_ERROR("uninstall failed: empty id");
        return false;
    }
    if (!getApp(id))
    {
        SERVICE_LOG_WARN("uninstall: app not installed: " << id);
        return false;
    }

    std::error_code ec;
    for (const std::string& dir : {getAppPackageDir(id), getAppStorageDir(id), getAppCacheDir(id)})
    {
        if (dir.empty())
        {
            continue;
        }
        const fs::path path = ucf::utilities::FilePathUtils::pathFromUtf8(dir);
        if (fs::exists(path, ec))
        {
            fs::remove_all(path, ec);
            if (ec)
            {
                SERVICE_LOG_ERROR("uninstall: failed to remove '" << dir << "': " << ec.message());
            }
        }
    }

    // Drop from the in-memory snapshot regardless of individual dir removal
    // results: the app is considered uninstalled once its package is gone.
    {
        std::lock_guard lock(mMutex);
        std::erase_if(mInstalledApps, [&id](const model::MiniAppManifest& app) { return app.id == id; });
    }

    SERVICE_LOG_INFO("uninstalled mini-app '" << id << "'");
    notifySink(&IMiniAppNotificationSink::onMiniAppUninstalled, id);
    return true;
}

std::string MiniAppManager::getAppPackageDir(const std::string& id) const
{
    if (id.empty())
    {
        return {};
    }
    const std::string root = packagesRoot();
    if (root.empty())
    {
        return {};
    }
    return ucf::utilities::FilePathUtils::utf8FromPath(
        ucf::utilities::FilePathUtils::joinPaths(ucf::utilities::FilePathUtils::pathFromUtf8(root), id));
}

std::string MiniAppManager::getAppStorageDir(const std::string& id) const
{
    if (id.empty())
    {
        return {};
    }
    const std::string root = storageRoot();
    if (root.empty())
    {
        return {};
    }
    const fs::path path =
        ucf::utilities::FilePathUtils::joinPaths(ucf::utilities::FilePathUtils::pathFromUtf8(root), id);
    ucf::utilities::FilePathUtils::EnsureDirectoryExists(path);
    return ucf::utilities::FilePathUtils::utf8FromPath(path);
}

std::string MiniAppManager::getAppCacheDir(const std::string& id) const
{
    if (id.empty())
    {
        return {};
    }
    const std::string root = cacheRoot();
    if (root.empty())
    {
        return {};
    }
    const fs::path path =
        ucf::utilities::FilePathUtils::joinPaths(ucf::utilities::FilePathUtils::pathFromUtf8(root), id);
    ucf::utilities::FilePathUtils::EnsureDirectoryExists(path);
    return ucf::utilities::FilePathUtils::utf8FromPath(path);
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish MiniAppManager Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
