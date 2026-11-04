#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace ucf::service::upgrade {

[[nodiscard]] bool hasSufficientDownloadSpace(const std::filesystem::path& directory, std::int64_t requiredBytes);

/// A cached installer must be a regular file, not a symbolic link, with exactly
/// the positive size declared by the manifest. SHA-256 verification is separate.
[[nodiscard]] bool isReusableInstallerFile(const std::filesystem::path& filePath, std::int64_t expectedBytes);

/// Returns an error description, or no value when the saved size is valid.
/// An unknown expected size still requires a nonempty file.
[[nodiscard]] std::optional<std::string> validateDownloadedFileSize(
    const std::filesystem::path& filePath, std::int64_t expectedBytes);

/// Removes one file and logs an error if removal fails. The caller determines
/// file ownership and schedules this operation after transport cancellation.
void removeDownloadFile(const std::filesystem::path& filePath, std::string_view failureContext);

} // namespace ucf::service::upgrade
