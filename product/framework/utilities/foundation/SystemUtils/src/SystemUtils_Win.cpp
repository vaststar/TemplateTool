#include "SystemUtils_Win.h"

#include <cstdlib>
#include <windows.h>

namespace ucf::utilities {

namespace {

std::filesystem::path getWideEnv(const wchar_t* name)
{
    const DWORD size = GetEnvironmentVariableW(name, nullptr, 0);
    if (size == 0)
    {
        return {};
    }

    std::wstring value(size, L'\0');
    const DWORD length = GetEnvironmentVariableW(name, value.data(), size);
    if (length == 0 || length >= size)
    {
        return {};
    }
    value.resize(length);
    return std::filesystem::path{value};
}

} // namespace

SystemColorScheme SystemUtils_Win::getSystemColorScheme()
{
    DWORD value = 1; // default: light
    DWORD size = sizeof(value);
    RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme",
        RRF_RT_REG_DWORD, nullptr, &value, &size);
    return (value == 0) ? SystemColorScheme::Dark : SystemColorScheme::Light;
}

std::filesystem::path SystemUtils_Win::getEnv(const std::string& name)
{
    if (const char* v = std::getenv(name.c_str()))
    {
        return std::filesystem::path{v};
    }
    return {};
}

std::filesystem::path SystemUtils_Win::getBaseStorageDir()
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

std::filesystem::path SystemUtils_Win::getBaseCacheDir()
{
    if (auto base = getWideEnv(L"LOCALAPPDATA"); !base.empty())
    {
        return base;
    }
    return getWideEnv(L"USERPROFILE");
}

}
