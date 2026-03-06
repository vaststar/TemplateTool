#include "SystemUtils_Mac.h"
#import <AppKit/AppKit.h>

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

}
