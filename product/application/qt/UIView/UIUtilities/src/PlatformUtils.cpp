#include <UIUtilities/PlatformUtils.h>

#include <windows.h>
#include "LoggerDefine.h"

namespace UIUtilities{
void PlatformUtils::openLinkInDefaultBrowser(const std::string& url, const std::string& params)
{
    ::ShellExecuteA(NULL, "open", url.c_str(), params.c_str(), NULL, SW_SHOW);
}
}