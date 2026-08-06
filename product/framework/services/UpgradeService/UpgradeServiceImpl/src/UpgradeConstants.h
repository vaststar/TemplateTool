#pragma once

/// Centralised string constants for the UpgradeService module.
/// All hardcoded directory names, file names, and URLs are defined here
/// so that changes only need to happen in one place.

namespace ucf::service::upgrade::constants {

// ── Directories ─────────────────────────────────────────────────────────────

/// Sub-directory under appCacheStoragePath for downloaded packages
inline constexpr auto kDownloadSubDir = "upgrade_downloads";

/// Temp directory used for the updater binary and marker file
inline constexpr auto kTempSubDir = "template-factory-upgrade";

/// Fallback app name used under system temp when appCacheStoragePath is unavailable
inline constexpr auto kTempFallbackAppName = "TemplateTool";

// ── File Names ──────────────────────────────────────────────────────────────

/// Default file name when the URL doesn't contain a filename
inline constexpr auto kDefaultPackageFileName = "upgrade_package.zip";

/// Marker file written before launching the updater (for crash recovery)
inline constexpr auto kUpgradeMarkerFileName = "upgrade_in_progress";

/// Updater binary name (platform-dependent)
#ifdef _WIN32
inline constexpr auto kUpdaterBinaryName = "updater.exe";
#else
inline constexpr auto kUpdaterBinaryName = "updater";
#endif

// ── Network ─────────────────────────────────────────────────────────────────

/// Default manifest URL for upgrade checks
inline constexpr auto kDefaultManifestUrl =
    "https://github.com/vaststar/TemplateTool/releases/latest/download/upgrade-manifest.json";

} // namespace ucf::service::upgrade::constants
