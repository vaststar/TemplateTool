#include "IPlatformCrashHandler.h"

#if defined(__APPLE__) || defined(__linux__)
#include "PosixCrashHandler.h"
#elif defined(_WIN32)
#include "WindowsCrashHandler.h"
#endif

namespace ucf::service {

void IPlatformCrashHandler::install()
{
    if (mInstalled)
    {
        return;
    }
    doInstall();
    mInstalled = true;
}

void IPlatformCrashHandler::uninstall()
{
    if (!mInstalled)
    {
        return;
    }
    doUninstall();
    mInstalled = false;
}

std::unique_ptr<IPlatformCrashHandler> IPlatformCrashHandler::create(Config config)
{
#if defined(__APPLE__) || defined(__linux__)
    return std::make_unique<PosixCrashHandler>(std::move(config));
#elif defined(_WIN32)
    return std::make_unique<WindowsCrashHandler>(std::move(config));
#else
    return nullptr;
#endif
}

} // namespace ucf::service
