#include "SystemUtils_Win.h"

#include <windows.h>

namespace ucf::utilities {

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

}
