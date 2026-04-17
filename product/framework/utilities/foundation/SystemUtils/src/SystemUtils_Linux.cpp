#include "SystemUtils_Linux.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace ucf::utilities {

SystemColorScheme SystemUtils_Linux::getSystemColorScheme()
{
    // GNOME / freedesktop portal: gsettings get org.gnome.desktop.interface color-scheme
    FILE* pipe = popen("gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null", "r");
    if (!pipe)
    {
        return SystemColorScheme::Light;
    }
    char buffer[64] = {};
    fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);
    return (strstr(buffer, "prefer-dark") != nullptr) ? SystemColorScheme::Dark : SystemColorScheme::Light;
}

std::filesystem::path SystemUtils_Linux::getEnv(const std::string& name)
{
    if (const char* v = std::getenv(name.c_str()))
    {
        return std::filesystem::path{v};
    }
    return {};
}

std::filesystem::path SystemUtils_Linux::getHome()
{
    if (const char* v = std::getenv("HOME"))
    {
        return std::filesystem::path{v};
    }
    return {};
}

std::filesystem::path SystemUtils_Linux::getBaseStorageDir()
{
    if (auto base = getEnv("XDG_DATA_HOME"); !base.empty())
    {
        return base;
    }
    if (auto home = getHome(); !home.empty())
    {
        return home / ".local" / "share";
    }
    return {};
}

std::filesystem::path SystemUtils_Linux::getBaseCacheDir()
{
    if (auto base = getEnv("XDG_CACHE_HOME"); !base.empty())
    {
        return base;
    }
    if (auto home = getHome(); !home.empty())
    {
        return home / ".cache";
    }
    return {};
}

}
