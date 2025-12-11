#pragma once

#include <string>
#include <UIUtilities/UIUtilitiesCommonFile/UIUtilitiesExport.h>

namespace UIUtilities{
class UIUtilities_EXPORT UIPlatformUtils final
{
public:
    static void openLinkInDefaultBrowser(const std::string& url, const std::string& params = {});
};
}
