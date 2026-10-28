#include "UpgradeValidation.h"
#include "UpdaterLog.h"
#include "../platform/PlatformUtils.h"

#include <system_error>

namespace updater {
namespace {

std::filesystem::path normalizeApplicationRoot(const std::filesystem::path& path)
{
    auto root = std::filesystem::weakly_canonical(std::filesystem::absolute(path)).lexically_normal();
    if (root != root.root_path() && root.filename().empty()) {
        root = root.parent_path();
    }
    return root;
}

bool pathsOverlap(const std::filesystem::path& first, const std::filesystem::path& second)
{
    // Compare filesystem identity to handle case and other directory aliases.
    const auto contains = [](const std::filesystem::path& ancestor, std::filesystem::path path) {
        while (!path.empty()) {
            std::error_code ec;
            if (std::filesystem::equivalent(ancestor, path, ec)) {
                return true;
            }
            if (ec && ec != std::errc::no_such_file_or_directory) {
                throw std::filesystem::filesystem_error("Cannot compare application directories", ancestor, path, ec);
            }
            const auto parent = path.parent_path();
            if (parent == path) {
                break;
            }
            path = parent;
        }
        return false;
    };
    return contains(first, second) || contains(second, first);
}

bool isNonEmptyFile(const std::filesystem::path& path)
{
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec) || ec) {
        return false;
    }
    const auto size = std::filesystem::file_size(path, ec);
    return !ec && size > 0;
}

} // namespace

std::optional<UpgradePaths> resolveUpgradePaths(
    const std::filesystem::path& staging,
    const std::filesystem::path& target)
{
    if (staging.empty() || target.empty()) {
        UPDATER_ERR("Staging and target directories are required");
        return std::nullopt;
    }

    const auto stagingDir = normalizeApplicationRoot(staging);
    const auto targetDir = normalizeApplicationRoot(target);
    if (stagingDir == stagingDir.root_path() || targetDir == targetDir.root_path()) {
        UPDATER_ERR("Application directories must not be filesystem roots");
        return std::nullopt;
    }

    auto backupDir = targetDir;
    backupDir += ".bak";
    if (pathsOverlap(stagingDir, targetDir) || pathsOverlap(stagingDir, backupDir)) {
        UPDATER_ERR("Staging must be separate from installation and backup directories");
        return std::nullopt;
    }
    return UpgradePaths{stagingDir, targetDir, backupDir};
}

bool validateApplicationLayout(const std::filesystem::path& root)
{
    const auto executable = platform::getApplicationExecutablePath(root);
    if (!isNonEmptyFile(executable)) {
        UPDATER_ERR("Application entry not found or empty: " << executable.string());
        return false;
    }
#ifdef __APPLE__
    if (!isNonEmptyFile(root / "Contents" / "Info.plist")) {
        UPDATER_ERR("Application bundle metadata not found or empty");
        return false;
    }
#endif
    return true;
}

} // namespace updater
