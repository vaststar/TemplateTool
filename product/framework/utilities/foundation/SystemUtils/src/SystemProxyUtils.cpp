#include <ucf/Utilities/SystemUtils/SystemProxyUtils.h>

#if defined(_WIN32)
#include "SystemProxyUtils_Win.h"
#elif defined(__APPLE__)
#include "SystemProxyUtils_Mac.h"
#elif defined(__linux__)
#include "SystemProxyUtils_Linux.h"
#endif

namespace ucf::utilities {

void SystemProxyUtils::enableHttpProxy(const std::string& host, int port)
{
#if defined(_WIN32)
    SystemProxyUtils_Win::enableHttpProxy(host, port);
#elif defined(__APPLE__)
    SystemProxyUtils_Mac::enableHttpProxy(host, port);
#elif defined(__linux__)
    SystemProxyUtils_Linux::enableHttpProxy(host, port);
#endif
}

void SystemProxyUtils::disableHttpProxy()
{
#if defined(_WIN32)
    SystemProxyUtils_Win::disableHttpProxy();
#elif defined(__APPLE__)
    SystemProxyUtils_Mac::disableHttpProxy();
#elif defined(__linux__)
    SystemProxyUtils_Linux::disableHttpProxy();
#endif
}

} // namespace ucf::utilities
