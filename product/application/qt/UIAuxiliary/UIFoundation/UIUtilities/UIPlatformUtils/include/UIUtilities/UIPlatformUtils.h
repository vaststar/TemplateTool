#pragma once

#include <string>
#include <UIUtilities/UIPlatformUtilsExport.h>

namespace UIUtilities{
class UIPlatformUtils_EXPORT UIPlatformUtils final
{
public:
    static void openLinkInDefaultBrowser(const std::string& url, const std::string& params = {});
    
    /// Reveal a file in the system file manager (Finder on macOS, Explorer on Windows)
    /// @param filePath Full path to the file to reveal
    static void revealFileInFinder(const std::string& filePath);
};
}
