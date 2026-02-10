#include <UIUtilities/UIPlatformUtils.h>

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#elif defined(__APPLE__)
#include <cstdlib>
#endif

#include "LoggerDefine.h"

namespace UIUtilities{
void UIPlatformUtils::openLinkInDefaultBrowser(const std::string& url, const std::string& params)
{
#ifdef WIN32
    ::ShellExecuteA(NULL, "open", url.c_str(), params.c_str(), NULL, SW_SHOW);
#elif defined(__APPLE__)
    std::string command = "open \"" + url + "\"";
    std::system(command.c_str());
#endif
}

void UIPlatformUtils::revealFileInFinder(const std::string& filePath)
{
    UIPlatformUtils_LOG_DEBUG("Revealing file in finder: " << filePath);
#ifdef WIN32
    std::string command = "/select,\"" + filePath + "\"";
    ::ShellExecuteA(NULL, "open", "explorer.exe", command.c_str(), NULL, SW_SHOW);
#elif defined(__APPLE__)
    std::string command = "open -R \"" + filePath + "\"";
    std::system(command.c_str());
#endif
}
}