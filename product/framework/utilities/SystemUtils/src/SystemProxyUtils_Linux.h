#pragma once

#include <string>

namespace ucf::utilities {

class SystemProxyUtils_Linux final
{
public:
    static void enableHttpProxy(const std::string& host, int port);
    static void disableHttpProxy();
};

} // namespace ucf::utilities
