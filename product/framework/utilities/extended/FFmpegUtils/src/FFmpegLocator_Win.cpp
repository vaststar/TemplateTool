#include "FFmpegLocatorPlatform.h"

#if defined(_WIN32)

#include <windows.h>

#include <cstdlib>
#include <filesystem>
#include <sstream>

namespace ucf::utilities::ffmpeg::detail {

namespace {
void dummyForModuleHandle() {}
} // namespace

std::string libraryDirectory()
{
    HMODULE hModule = nullptr;
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&dummyForModuleHandle), &hModule))
    {
        return {};
    }

    char path[MAX_PATH] = {};
    DWORD len = GetModuleFileNameA(hModule, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
    {
        return {};
    }

    std::string fullPath(path, len);
    auto lastSep = fullPath.find_last_of("\\/");
    return (lastSep != std::string::npos) ? fullPath.substr(0, lastSep) : ".";
}

std::string executableName(const std::string& base)
{
    return base + ".exe";
}

std::vector<std::string> candidatePaths(const std::string& exeName, const std::string& libDir)
{
    if (libDir.empty())
    {
        return {};
    }
    return {
        libDir + "/" + exeName,
        libDir + "/../bin/" + exeName,
        libDir + "/ffmpeg/" + exeName,
    };
}

std::vector<std::string> candidatePathsForAppDir(const std::string& exeName, const std::string& appDir)
{
    return {
        appDir + "/" + exeName,
        appDir + "/ffmpeg/" + exeName,
    };
}

bool isExecutableFile(const std::string& path)
{
    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec);
}

std::string findInPath(const std::string& exeName)
{
    char* pathEnv = nullptr;
    std::size_t pathLen = 0;
    if (_dupenv_s(&pathEnv, &pathLen, "PATH") != 0 || pathEnv == nullptr)
    {
        return {};
    }
    std::string pathValue(pathEnv);
    std::free(pathEnv);
    std::istringstream stream(pathValue);
    std::string dir;
    while (std::getline(stream, dir, ';'))
    {
        if (dir.empty())
        {
            continue;
        }
        auto candidate = std::filesystem::path(dir) / exeName;
        std::error_code ec;
        if (std::filesystem::is_regular_file(candidate, ec))
        {
            auto canonical = std::filesystem::canonical(candidate, ec);
            if (!ec)
            {
                return canonical.string();
            }
        }
    }
    return {};
}

} // namespace ucf::utilities::ffmpeg::detail

#endif // _WIN32
