#pragma once

#include <ucf/Utilities/SystemUtils/SystemUtils.h>

#include <filesystem>
#include <string>

namespace ucf::utilities {

class SystemUtils_Linux final
{
public:
    static SystemColorScheme getSystemColorScheme();
    static std::filesystem::path getBaseStorageDir();
    static std::filesystem::path getBaseCacheDir();
private:
    static std::filesystem::path getEnv(const std::string& name);
    static std::filesystem::path getHome();
};

}
