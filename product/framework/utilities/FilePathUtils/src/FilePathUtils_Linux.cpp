#include "FilePathUtils_Linux.h"
#include <cstdlib>

namespace ucf::utilities {

std::filesystem::path FilePathUtils_Linux::getHome()
{
    if (const char* v = std::getenv("HOME"))
    {
        return std::filesystem::path{v};
    }
    return {};
}

std::filesystem::path FilePathUtils_Linux::getEnv(const std::string& name)
{
    if (const char* v = std::getenv(name.c_str()))
    {
        return std::filesystem::path{v};
    }
    return {};
}

std::filesystem::path FilePathUtils_Linux::getBaseStorageDir()
{
    if (auto base = getEnv("XDG_DATA_HOME"); !base.empty())
    {
        return base;
    }

    if (auto home = getHome(); !home.empty())
    {
        return FilePathUtils::joinPaths(home, ".local", "share");
    }

    return {};
}

} // namespace ucf::utilities