#include "IPlatformHangHandler.h"

#if defined(__APPLE__)
#include "platform/MacOSHangHandler.h"
#elif defined(__linux__)
#include "platform/LinuxHangHandler.h"
#elif defined(_WIN32)
#include "platform/WindowsHangHandler.h"
#endif

namespace ucf::service {

std::unique_ptr<IPlatformHangHandler> IPlatformHangHandler::create()
{
#if defined(__APPLE__)
    return std::make_unique<MacOSHangHandler>();
#elif defined(__linux__)
    return std::make_unique<LinuxHangHandler>();
#elif defined(_WIN32)
    return std::make_unique<WindowsHangHandler>();
#else
    return nullptr;
#endif
}

} // namespace ucf::service
