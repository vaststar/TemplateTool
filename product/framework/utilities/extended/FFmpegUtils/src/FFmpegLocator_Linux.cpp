#include "FFmpegLocatorPlatform.h"

#if defined(__linux__)

#include <climits>
#include <cstdlib>
#include <dlfcn.h>
#include <filesystem>
#include <sstream>
#include <unistd.h>

namespace ucf::utilities::ffmpeg::detail {

std::string libraryDirectory()
{
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(&libraryDirectory), &info) == 0 || info.dli_fname == nullptr)
    {
        return {};
    }

    std::string fullPath = info.dli_fname;
    if (!fullPath.empty() && fullPath[0] != '/')
    {
        char resolved[PATH_MAX] = {};
        if (realpath(fullPath.c_str(), resolved))
        {
            fullPath = resolved;
        }
    }

    auto lastSep = fullPath.rfind('/');
    return (lastSep != std::string::npos) ? fullPath.substr(0, lastSep) : ".";
}

std::string executableName(const std::string& base)
{
    return base;
}

std::vector<std::string> candidatePaths(const std::string& exeName, const std::string& libDir)
{
    std::vector<std::string> paths;
    if (!libDir.empty())
    {
        paths = {
            libDir + "/" + exeName,
            libDir + "/../bin/" + exeName,
            libDir + "/../lib/" + exeName,
        };
    }
    paths.push_back("/usr/bin/" + exeName);
    paths.push_back("/usr/local/bin/" + exeName);
    return paths;
}

bool isExecutableFile(const std::string& path)
{
    return access(path.c_str(), X_OK) == 0;
}

std::string findInPath(const std::string& exeName)
{
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv)
    {
        return {};
    }
    std::istringstream stream(pathEnv);
    std::string dir;
    while (std::getline(stream, dir, ':'))
    {
        if (dir.empty())
        {
            continue;
        }
        auto candidate = std::filesystem::path(dir) / exeName;
        std::error_code ec;
        if (std::filesystem::is_regular_file(candidate, ec) && access(candidate.c_str(), X_OK) == 0)
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

#endif // __linux__
