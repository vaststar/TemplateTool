#pragma once

#include <string>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>
namespace ucf::utilities{
class FilePathUtils_Win final
{
public:
    // Windows: %LOCALAPPDATA%
    static std::filesystem::path getBaseStorageDir();
private:
    static std::filesystem::path getEnv(const std::string& name);
};
}