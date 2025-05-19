#include <UIUtilities/PlatformUtils.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "LoggerDefine.h"

namespace UIUtilities{
void PlatformUtils::openLinkInDefaultBrowser(const std::string& url, const std::string& params)
{
#ifdef WIN32
    ::ShellExecuteA(NULL, "open", url.c_str(), params.c_str(), NULL, SW_SHOW);
#endif
}
}