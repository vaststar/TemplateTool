#pragma once

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

#include <filesystem>

namespace ucf::utilities {

enum class SystemColorScheme { Light, Dark };

class Utilities_EXPORT SystemUtils final
{
public:
    static SystemColorScheme getSystemColorScheme();

    /// Platform base directory for persistent application data.
    /// Windows: %LOCALAPPDATA%
    /// macOS:   ~/Library/Application Support
    /// Linux:   $XDG_DATA_HOME or ~/.local/share
    static std::filesystem::path getBaseStorageDir();

    /// Platform base directory for cache data.
    /// Windows: %LOCALAPPDATA%
    /// macOS:   ~/Library/Caches
    /// Linux:   $XDG_CACHE_HOME or ~/.cache
    static std::filesystem::path getBaseCacheDir();
};

}
