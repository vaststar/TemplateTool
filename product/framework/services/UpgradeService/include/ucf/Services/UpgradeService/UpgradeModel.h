#pragma once

#include <cstdint>
#include <string>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service::model {

enum class UpgradeState {
    Idle,               ///< No upgrade activity
    Checking,           ///< Checking for available upgrade
    UpgradeAvailable,   ///< A newer version is available
    Downloading,        ///< Downloading upgrade package
    Verifying,          ///< Verifying package integrity (SHA-256)
    ReadyToInstall,     ///< Package verified, ready to install
    Installing,         ///< Launching updater process
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
    LaunchUpdaterFailed,    ///< Failed to launch updater process
    Cancelled               ///< User cancelled the operation
};

struct SERVICE_EXPORT PackageInfo {
    std::string downloadUrl;
    std::string sha256;
    int64_t sizeBytes{0};
};

struct SERVICE_EXPORT UpgradeInfo {
    std::string version;
    std::string releaseDate;
    std::string releaseNotes;
    PackageInfo package;
    bool mandatory{false};
    std::string minVersion;
};

struct SERVICE_EXPORT UpgradeCheckResult {
    bool hasUpgrade{false};
    UpgradeInfo upgradeInfo;
};

} // namespace ucf::service::model
