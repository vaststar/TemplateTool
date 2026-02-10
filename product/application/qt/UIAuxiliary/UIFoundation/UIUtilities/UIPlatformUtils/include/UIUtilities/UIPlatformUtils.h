#pragma once

#include <string>
#include <UIUtilities/UIUtilitiesCommonFile/UIUtilitiesExport.h>

namespace UIUtilities{
class UIUtilities_EXPORT UIPlatformUtils final
{
public:
    static void openLinkInDefaultBrowser(const std::string& url, const std::string& params = {});
    
    /// Reveal a file in the system file manager (Finder on macOS, Explorer on Windows)
    /// @param filePath Full path to the file to reveal
    static void revealFileInFinder(const std::string& filePath);
};
}
