#pragma once

#include <ucf/Utilities/SystemUtils/SystemUtils.h>

#include <filesystem>

namespace ucf::utilities {

class SystemUtils_Mac final
{
public:
    static SystemColorScheme getSystemColorScheme();
    static std::filesystem::path getBaseStorageDir();
    static std::filesystem::path getBaseCacheDir();
private:
    static std::filesystem::path getHome();
};

}
