#include "SystemUtils_Mac.h"

#include <cstdlib>
#include <cstring>

namespace ucf::utilities {

SystemColorScheme SystemUtils_Mac::getSystemColorScheme()
{
    // Read macOS appearance setting via defaults command
    FILE* pipe = popen("defaults read -g AppleInterfaceStyle 2>/dev/null", "r");
    if (!pipe)
    {
        return SystemColorScheme::Light;
    }
    char buffer[64] = {};
    fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);
    return (strncmp(buffer, "Dark", 4) == 0) ? SystemColorScheme::Dark : SystemColorScheme::Light;
}

}
