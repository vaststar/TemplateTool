#pragma once

/// Centralised string constants for the UpgradeService module.
/// All hardcoded directory names, file names, and URLs are defined here
/// so that changes only need to happen in one place.

namespace ucf::service::upgrade::constants {

// ── Directories ─────────────────────────────────────────────────────────────

/// Sub-directory under appCacheStoragePath for downloaded packages
inline constexpr auto kDownloadSubDir = "upgrade_downloads";

/// Fallback app name used under system temp when appCacheStoragePath is unavailable
inline constexpr auto kTempFallbackAppName = "TemplateTool";

// ── File Names ──────────────────────────────────────────────────────────────

/// Written into the application root by Linux system packages.
inline constexpr auto kPackageFormatMarkerFileName = ".upgrade-package-format";

// ── Network ─────────────────────────────────────────────────────────────────

/// Default manifest URL for upgrade checks
inline constexpr auto kDefaultManifestUrl =
    "https://github.com/vaststar/TemplateTool/releases/latest/download/upgrade-manifest.json";

} // namespace ucf::service::upgrade::constants
