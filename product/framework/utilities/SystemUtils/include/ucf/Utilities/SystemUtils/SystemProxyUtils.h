#pragma once

#include <string>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

/// Cross-platform HTTP(S) system proxy configuration.
///
/// On Windows: writes to Internet Settings registry keys.
/// On macOS:   calls networksetup for Wi-Fi / Ethernet interfaces.
/// On Linux:   sets GNOME gsettings (best-effort).
class Utilities_EXPORT SystemProxyUtils final
{
public:
    /// Enable the OS-level HTTP and HTTPS proxy pointing at host:port.
    static void enableHttpProxy(const std::string& host, int port);

    /// Disable (restore to "direct") the OS-level HTTP/HTTPS proxy.
    static void disableHttpProxy();
};

} // namespace ucf::utilities
