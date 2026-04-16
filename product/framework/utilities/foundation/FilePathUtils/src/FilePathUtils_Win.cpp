#include "FilePathUtils_Win.h"
#include <windows.h>

namespace ucf::utilities {

std::filesystem::path FilePathUtils_Win::getEnv(const std::string& name)
{
    if (const char* v = std::getenv(name.c_str()))
    {
        return std::filesystem::path{v};
    }
    return {};
}

std::filesystem::path FilePathUtils_Win::getBaseStorageDir()
{
    
    if (auto base = getEnv("LOCALAPPDATA"); !base.empty())
    {
        return base;
    }

    if (auto profile = getEnv("USERPROFILE"); !profile.empty())
    {
        return profile;
    }
    
    return {};
}

} // namespace ucf::utilities
