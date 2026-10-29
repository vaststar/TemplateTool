#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <ucf/services/UpgradeService/UpgradeServiceTypesExport.h>

namespace ucf::service::model {

enum class UpgradeState {
    Idle,               ///< No upgrade activity
    Checking,           ///< Checking for available upgrade
    UpgradeAvailable,   ///< A newer version is available
    Downloading,        ///< Downloading upgrade package
    Verifying,          ///< Verifying package integrity (SHA-256)
    ReadyToInstall,     ///< Installer downloaded and verified
    OpeningInstaller,   ///< Waiting for the desktop to accept the installer
    InstallerOpened,    ///< Open request accepted; installation is not monitored
    Failed              ///< An error occurred
};

enum class UpgradeErrorCode {
    None = 0,
    NetworkError,           ///< HTTP request failed
    ServerError,            ///< Server returned error status
    ParseError,             ///< Failed to parse server response
    DownloadFailed,         ///< Download failed after retries
    VerifyFailed,           ///< SHA-256 mismatch
    DiskSpaceError,         ///< Insufficient disk space
    OpenInstallerFailed,    ///< Desktop rejected the installer open request
    UnsupportedInstallation, ///< No installer format is known for this installation
    Cancelled               ///< User cancelled the operation
};

enum class PackageFormat { Unknown, Exe, Dmg, Deb, Rpm };

/// Manifest format key and filename extension (without the dot).
[[nodiscard]] UPGRADE_SERVICE_TYPES_API std::string_view packageFormatName(PackageFormat format);

struct UPGRADE_SERVICE_TYPES_API PackageInfo {
    std::string downloadUrl;
    std::string sha256;
    int64_t sizeBytes{0};
    PackageFormat format{PackageFormat::Unknown};
};

struct UPGRADE_SERVICE_TYPES_API UpgradeInfo {
    std::string version;
    std::string releaseDate;
    std::string releaseNotes;
    PackageInfo package;
    bool mandatory{false};
    std::string minVersion;
};

struct UPGRADE_SERVICE_TYPES_API UpgradeCheckResult {
    bool hasUpgrade{false};
    UpgradeInfo upgradeInfo;
};

} // namespace ucf::service::model
