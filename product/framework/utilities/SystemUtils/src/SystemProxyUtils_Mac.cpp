#include "SystemProxyUtils_Mac.h"

#include <cstdlib>
#include <string>

namespace ucf::utilities {

void SystemProxyUtils_Mac::enableHttpProxy(const std::string& host, int port)
{
    std::string portStr = std::to_string(port);
    const char* services[] = {"Wi-Fi", "Ethernet", nullptr};

    for (int i = 0; services[i] != nullptr; ++i)
    {
        std::string svc = services[i];
        std::system(("networksetup -setwebproxy \""
                      + svc + "\" " + host + " " + portStr + " 2>/dev/null").c_str());
        std::system(("networksetup -setsecurewebproxy \""
                      + svc + "\" " + host + " " + portStr + " 2>/dev/null").c_str());
        std::system(("networksetup -setwebproxystate \""
                      + svc + "\" on 2>/dev/null").c_str());
        std::system(("networksetup -setsecurewebproxystate \""
                      + svc + "\" on 2>/dev/null").c_str());
    }
}

void SystemProxyUtils_Mac::disableHttpProxy()
{
    const char* services[] = {"Wi-Fi", "Ethernet", nullptr};

    for (int i = 0; services[i] != nullptr; ++i)
    {
        std::string svc = services[i];
        std::system(("networksetup -setwebproxystate \""
                      + svc + "\" off 2>/dev/null").c_str());
        std::system(("networksetup -setsecurewebproxystate \""
                      + svc + "\" off 2>/dev/null").c_str());
    }
}

} // namespace ucf::utilities
