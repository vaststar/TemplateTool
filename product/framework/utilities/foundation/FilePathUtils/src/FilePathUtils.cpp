
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace ucf::utilities {

// ============================================================================
// Encoding bridge — internal helpers (Windows only)
// ============================================================================

#ifdef _WIN32

static std::wstring utf8ToWide(std::string_view utf8)
{
    if (utf8.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    if (len <= 0) return {};
    std::wstring wide(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), wide.data(), len);
    return wide;
}

static std::string wideToUtf8(const std::wstring& wide)
{
    if (wide.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string utf8(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), utf8.data(), len, nullptr, nullptr);
    return utf8;
}

#endif

// ============================================================================
// Encoding bridge — public API
// ============================================================================

std::filesystem::path FilePathUtils::pathFromUtf8(std::string_view utf8)
{
    if (utf8.empty())
    {
        return {};
    }
#ifdef _WIN32
    return std::filesystem::path(utf8ToWide(utf8));
#else
    return std::filesystem::path(std::string(utf8));
#endif
}

std::string FilePathUtils::utf8FromPath(const std::filesystem::path& p)
{
    if (p.empty())
    {
        return {};
    }
#ifdef _WIN32
    return wideToUtf8(p.wstring());
#else
    return p.string();
#endif
}

std::filesystem::path FilePathUtils::canonicalUtf8(std::string_view utf8)
{
    std::error_code ec;
    auto result = std::filesystem::canonical(pathFromUtf8(utf8), ec);
    if (ec)
    {
        return {};
    }
    return result;
}

std::filesystem::path FilePathUtils::absoluteUtf8(std::string_view utf8)
{
    std::error_code ec;
    auto result = std::filesystem::absolute(pathFromUtf8(utf8), ec);
    if (ec)
    {
        return {};
    }
    return result;
}

bool FilePathUtils::existsUtf8(std::string_view utf8)
{
    std::error_code ec;
    return std::filesystem::exists(pathFromUtf8(utf8), ec);
}

bool FilePathUtils::createDirectoriesUtf8(std::string_view utf8)
{
    std::error_code ec;
    std::filesystem::create_directories(pathFromUtf8(utf8), ec);
    return !ec;
}

// ============================================================================
// Directory utilities
// ============================================================================

bool FilePathUtils::EnsureDirectoryExists(const std::filesystem::path& path)
{
    std::filesystem::path dir;
    if (path.has_extension())
    {
        dir = path.parent_path();
    }
    else
    {
        dir = path;
    }

    if (dir.empty())
    {
        dir = std::filesystem::path(".");
    }

    if (std::error_code ec; std::filesystem::exists(dir, ec))
    {
        return std::filesystem::is_directory(dir, ec);
    }
    else
    {
        return std::filesystem::create_directories(dir, ec);
    }
}
}
