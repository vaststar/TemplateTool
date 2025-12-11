#pragma once

#include <string>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>

namespace ucf::utilities{
class FilePathUtils_Mac final
{
public:
    // Windows: %LOCALAPPDATA%
    static std::filesystem::path getBaseStorageDir();
private:
    static std::filesystem::path getHome();
};
}