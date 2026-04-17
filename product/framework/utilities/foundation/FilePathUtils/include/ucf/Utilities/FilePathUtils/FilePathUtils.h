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
    // === Encoding-safe path construction (UTF-8 ↔ fs::path) ===

    /// Construct a filesystem path from a UTF-8 encoded string.
    /// On Windows, converts to wide string first to avoid codepage issues.
    /// On Unix, passes through directly (locale is assumed UTF-8).
    [[nodiscard]] static std::filesystem::path pathFromUtf8(std::string_view utf8);

    /// Extract a UTF-8 encoded string from a filesystem path.
    /// On Windows, converts from wide string. On Unix, returns .string() directly.
    [[nodiscard]] static std::string utf8FromPath(const std::filesystem::path& p);

    // === Convenience wrappers (accept UTF-8, return path or UTF-8) ===

    /// fs::canonical on a UTF-8 path string. Returns empty path on error.
    [[nodiscard]] static std::filesystem::path canonicalUtf8(std::string_view utf8);

    /// fs::absolute on a UTF-8 path string. Returns empty path on error.
    [[nodiscard]] static std::filesystem::path absoluteUtf8(std::string_view utf8);

    /// fs::exists on a UTF-8 path string.
    [[nodiscard]] static bool existsUtf8(std::string_view utf8);

    /// fs::create_directories on a UTF-8 path string.
    [[nodiscard]] static bool createDirectoriesUtf8(std::string_view utf8);

    // === Directory utilities ===

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
