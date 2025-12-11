#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <type_traits>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {
template <typename T>
concept PathLike = std::is_same_v<std::decay_t<T>, std::filesystem::path>;

template <typename T>
concept StringLike =
    std::is_same_v<std::decay_t<T>, std::string> ||
    std::is_convertible_v<T, std::string_view> ||
    std::is_same_v<std::decay_t<T>, const char*> ||
    std::is_same_v<std::decay_t<T>, char*>;

class Utilities_EXPORT FilePathUtils {
public:
    // Windows: %LOCALAPPDATA%
    // macOS:   ~/Library/Application Support
    // Linux:   $XDG_DATA_HOME or ~/.local/share
    static std::filesystem::path getBaseStorageDir();

    static bool EnsureDirectoryExists(const std::filesystem::path& path);


    template <PathLike First, StringLike... Rest>
    static std::filesystem::path joinPaths(First&& first, Rest&&... rest)
    {
        std::filesystem::path p{ std::forward<First>(first) };
        (p /= ... /= rest);
        return p;
    }
};

}  // namespace ucf::utilities
