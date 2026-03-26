#include "SystemProxyUtils_Win.h"

#include <windows.h>
#include <wininet.h>
#include <string>

namespace ucf::utilities {

void SystemProxyUtils_Win::enableHttpProxy(const std::string& host, int port)
{
    std::string proxyServer = host + ":" + std::to_string(port);

    // ProxyEnable = 1
    DWORD enable = 1;
    RegSetKeyValueA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
        "ProxyEnable", REG_DWORD, &enable, sizeof(enable));

    // ProxyServer = "host:port"
    RegSetKeyValueA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
        "ProxyServer", REG_SZ, proxyServer.c_str(),
        static_cast<DWORD>(proxyServer.size() + 1));

    // Notify WinINet that proxy settings changed
    InternetSetOptionA(nullptr, INTERNET_OPTION_SETTINGS_CHANGED, nullptr, 0);
    InternetSetOptionA(nullptr, INTERNET_OPTION_REFRESH, nullptr, 0);
}

void SystemProxyUtils_Win::disableHttpProxy()
{
    DWORD enable = 0;
    RegSetKeyValueA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
        "ProxyEnable", REG_DWORD, &enable, sizeof(enable));

    InternetSetOptionA(nullptr, INTERNET_OPTION_SETTINGS_CHANGED, nullptr, 0);
    InternetSetOptionA(nullptr, INTERNET_OPTION_REFRESH, nullptr, 0);
}

} // namespace ucf::utilities
