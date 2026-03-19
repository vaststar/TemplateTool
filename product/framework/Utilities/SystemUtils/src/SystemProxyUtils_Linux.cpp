#include "SystemProxyUtils_Linux.h"

#include <cstdlib>
#include <string>

namespace ucf::utilities {

void SystemProxyUtils_Linux::enableHttpProxy(const std::string& host, int port)
{
    std::string portStr = std::to_string(port);

    // GNOME (gsettings) — best-effort; no-op on non-GNOME desktops
    std::system("gsettings set org.gnome.system.proxy mode 'manual' 2>/dev/null");
    std::system(("gsettings set org.gnome.system.proxy.http host '"
                  + host + "' 2>/dev/null").c_str());
    std::system(("gsettings set org.gnome.system.proxy.http port "
                  + portStr + " 2>/dev/null").c_str());
    std::system(("gsettings set org.gnome.system.proxy.https host '"
                  + host + "' 2>/dev/null").c_str());
    std::system(("gsettings set org.gnome.system.proxy.https port "
                  + portStr + " 2>/dev/null").c_str());
}

void SystemProxyUtils_Linux::disableHttpProxy()
{
    std::system("gsettings set org.gnome.system.proxy mode 'none' 2>/dev/null");
}

} // namespace ucf::utilities
