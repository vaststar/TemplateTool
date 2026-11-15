#pragma once

#include <string>
#include <UIUtilities/UIPlatformUtilsExport.h>

namespace UIUtilities{
class UIPlatformUtils_EXPORT UIPlatformUtils final
{
public:
    /// Strings use UTF-8. params is passed through on Windows and ignored elsewhere.
    static void openLinkInDefaultBrowser(const std::string& url, const std::string& params = {});
    
    /// Reveal a file in Finder / Explorer; Linux opens its parent directory.
    /// @param filePath UTF-8 full path to the file to reveal
    static void revealFileInFinder(const std::string& filePath);
};
}
