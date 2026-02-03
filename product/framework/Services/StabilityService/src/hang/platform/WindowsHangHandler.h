#pragma once

#if defined(_WIN32)

#include "../IPlatformHangHandler.h"

namespace ucf::service {

/**
 * @brief Windows implementation of the platform hang handler
 *
 * Uses Windows APIs to capture thread stack traces
 */
class WindowsHangHandler : public IPlatformHangHandler
{
public:
    WindowsHangHandler();
    ~WindowsHangHandler() override = default;

    std::string captureMainThreadStack(std::thread::id mainThreadId) const override;
    std::string captureCurrentThreadStack(int skipFrames = 0) const override;
    bool isMainThreadCaptureSupported() const override;

private:
    std::string captureThreadStack(void* threadHandle, int skipFrames) const;
    std::string demangleSymbol(const char* symbol) const;
};

} // namespace ucf::service

#endif // defined(_WIN32)
