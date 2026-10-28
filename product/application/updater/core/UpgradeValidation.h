#pragma once

#include <filesystem>
#include <optional>

namespace updater {

struct UpgradePaths {
    std::filesystem::path staging;
    std::filesystem::path target;
    std::filesystem::path backup;
};

// Resolve paths before changing the working directory and reject roots or
// overlapping directories. Invalid input returns nullopt; filesystem errors
// propagate to the executor's exception handler. No files are modified.
std::optional<UpgradePaths> resolveUpgradePaths(
    const std::filesystem::path& staging,
    const std::filesystem::path& target);

// Check the platform entry file and required bundle metadata, without launching.
bool validateApplicationLayout(const std::filesystem::path& root);

} // namespace updater
