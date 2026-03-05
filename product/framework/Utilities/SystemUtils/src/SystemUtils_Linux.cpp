#include "SystemUtils_Linux.h"

#include <cstdio>
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

}
