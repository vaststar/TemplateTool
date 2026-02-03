#include "IPlatformHangHandler.h"

#if defined(__APPLE__) || defined(__linux__)
#include "PosixHangHandler.h"
#elif defined(_WIN32)
#include "WindowsHangHandler.h"
#endif

namespace ucf::service {

std::unique_ptr<IPlatformHangHandler> IPlatformHangHandler::create()
{
#if defined(__APPLE__) || defined(__linux__)
    return std::make_unique<PosixHangHandler>();
#elif defined(_WIN32)
    return std::make_unique<WindowsHangHandler>();
#else
    return nullptr;
#endif
}

} // namespace ucf::service
