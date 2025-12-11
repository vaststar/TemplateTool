
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>
#include <thread>

#if defined(_WIN32)
#include "FilePathUtils_Win.h"
#elif defined(__APPLE__)
#include "FilePathUtils_Mac.h"
#elif defined(__linux__)
#include "FilePathUtils_Linux.h"
#endif

namespace ucf::utilities{
std::filesystem::path FilePathUtils::getBaseStorageDir()
{
    #if defined(_WIN32)
        return FilePathUtils_Win::getBaseStorageDir();
    #elif defined(__APPLE__)
        return FilePathUtils_Mac::getBaseStorageDir();
    #elif defined(__linux__)
        return FilePathUtils_Linux::getBaseStorageDir();
    #endif
        return {};
}

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
