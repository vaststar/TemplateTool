#pragma once

#include "IPlatformHangHandler.h"

#if defined(__APPLE__) || defined(__linux__)

namespace ucf::service {

class PosixHangHandler : public IPlatformHangHandler
{
public:
    PosixHangHandler();
    ~PosixHangHandler() override = default;
    
    std::string captureMainThreadStack(std::thread::id mainThreadId) const override;
    std::string captureCurrentThreadStack(int skipFrames = 0) const override;
    [[nodiscard]] bool isMainThreadCaptureSupported() const override;

private:
    // Helper to demangle C++ symbol names
    std::string demangleSymbol(const char* symbol) const;
};

} // namespace ucf::service

#endif // defined(__APPLE__) || defined(__linux__)
