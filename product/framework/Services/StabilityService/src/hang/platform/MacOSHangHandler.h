#pragma once

#include "IPlatformHangHandler.h"

#if defined(__APPLE__)

#include <pthread.h>

namespace ucf::service {

class MacOSHangHandler : public IPlatformHangHandler
{
public:
    MacOSHangHandler();
    ~MacOSHangHandler() override = default;
    
    std::string captureMainThreadStack(std::thread::id mainThreadId) const override;
    std::string captureCurrentThreadStack(int skipFrames = 0) const override;
    [[nodiscard]] bool isMainThreadCaptureSupported() const override;

private:
    std::string captureMachThreadStack(pthread_t targetThread) const;
    std::string demangleSymbol(const char* symbol) const;
    
    pthread_t mMainThreadPthread{0};
};

} // namespace ucf::service

#endif // defined(__APPLE__)
