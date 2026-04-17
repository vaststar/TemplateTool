#pragma once

#include <string>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>
namespace ucf::utilities{
class FilePathUtils_Linux final
{
public:
    // Linux: $XDG_DATA_HOME or ~/.local/share
    static std::filesystem::path getBaseStorageDir();

    // Linux: $XDG_CACHE_HOME or ~/.cache
    static std::filesystem::path getBaseCacheDir();
private:
    static std::filesystem::path getEnv(const std::string& name);
    static std::filesystem::path getHome();
};
}
