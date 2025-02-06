#pragma once

#include <string>
#include <UIUtilities/UIUtilitiesExport.h>

namespace UIUtilities{
class UIUtilities_EXPORT PlatformUtils final
{
public:
    static void openLinkInDefaultBrowser(const std::string& url, const std::string& params = {});
};
}
