#pragma once

#include <string>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>
namespace ucf::utilities{
class FilePathUtils_Win final
{
public:
    // Windows: %LOCALAPPDATA%
    static std::filesystem::path getBaseStorageDir();

    // Windows: %LOCALAPPDATA% (same root, Cache subfolder appended by caller)
    static std::filesystem::path getBaseCacheDir();
private:
    static std::filesystem::path getEnv(const std::string& name);
};
}
