#pragma once

#include <string>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>

namespace ucf::utilities{
class FilePathUtils_Mac final
{
public:
    // macOS: ~/Library/Application Support
    static std::filesystem::path getBaseStorageDir();

    // macOS: ~/Library/Caches
    static std::filesystem::path getBaseCacheDir();
private:
    static std::filesystem::path getHome();
};
}
