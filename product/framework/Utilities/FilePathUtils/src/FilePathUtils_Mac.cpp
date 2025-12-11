#include "FilePathUtils_Mac.h"
#include <cstdlib>

namespace ucf::utilities {

std::filesystem::path FilePathUtils_Mac::getHome()
{
    if (const char* v = std::getenv("HOME"))
    {
        return std::filesystem::path{v};
    }
    return {};
}

std::filesystem::path FilePathUtils_Mac::getBaseStorageDir()
{
    if (auto home = getHome(); !home.empty())
    {
        return FilePathUtils::joinPaths(home, "Library", "Application Support");
    }
    return {};
}

} // namespace ucf::utilities
