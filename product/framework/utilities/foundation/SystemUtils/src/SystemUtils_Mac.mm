#include "SystemUtils_Mac.h"
#import <AppKit/AppKit.h>

#include <cstdlib>

namespace ucf::utilities {

SystemColorScheme SystemUtils_Mac::getSystemColorScheme()
{
    @autoreleasepool {
        NSAppearance* appearance = [NSAppearance currentDrawingAppearance];
        if (!appearance) {
            appearance = [NSApp effectiveAppearance];
        }
        if (!appearance) {
            return SystemColorScheme::Light;
        }

        NSAppearanceName bestMatch = [appearance bestMatchFromAppearancesWithNames:@[
            NSAppearanceNameAqua,
            NSAppearanceNameDarkAqua
        ]];

        return [bestMatch isEqualToString:NSAppearanceNameDarkAqua]
            ? SystemColorScheme::Dark
            : SystemColorScheme::Light;
    }
}

std::filesystem::path SystemUtils_Mac::getHome()
{
    if (const char* v = std::getenv("HOME"))
    {
        return std::filesystem::path{v};
    }
    return {};
}

std::filesystem::path SystemUtils_Mac::getBaseStorageDir()
{
    if (auto home = getHome(); !home.empty())
    {
        return home / "Library" / "Application Support";
    }
    return {};
}

std::filesystem::path SystemUtils_Mac::getBaseCacheDir()
{
    if (auto home = getHome(); !home.empty())
    {
        return home / "Library" / "Caches";
    }
    return {};
}

}
