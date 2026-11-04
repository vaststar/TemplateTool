#pragma once

#include <ucf/services/UpgradeService/UpgradeModel.h>

#include <filesystem>
#include <optional>

namespace ucf::service::upgrade {

struct CompletedInstaller
{
    model::PackageInfo packageInfo;
    std::filesystem::path filePath;
};

[[nodiscard]] bool sameInstallerContent(const model::PackageInfo& lhs, const model::PackageInfo& rhs) noexcept;

/// Returns a path hint matching the current manifest. The caller must still
/// check the file and verify its SHA-256 before allowing installation.
[[nodiscard]] std::optional<CompletedInstaller> readCompletedInstallerRecord(
    const std::filesystem::path& directory, const model::PackageInfo& expected);

/// Called on the download manager's I/O worker. An empty installer removes only
/// its record; it never removes an installer package.
void writeCompletedInstallerRecord(
    const std::filesystem::path& directory, const std::optional<CompletedInstaller>& installer);

} // namespace ucf::service::upgrade
