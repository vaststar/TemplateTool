#pragma once

#if defined(__APPLE__) || defined(__linux__)

#include "../IPlatformCrashHandler.h"

#include <signal.h>

namespace ucf::service {

class PosixCrashHandler final : public IPlatformCrashHandler
{
public:
    /// Construct and install signal handler
    explicit PosixCrashHandler(Config config);
    
    /// Auto-uninstall on destruction
    ~PosixCrashHandler() override;
    
    // IPlatformCrashHandler interface implementation
    std::string captureStackTrace(int skipFrames = 0) const override;
    
protected:
    // Base class pure virtual function implementation
    void doInstall() override;
    void doUninstall() override;
    
private:
    static void signalHandler(int sig, siginfo_t* info, void* context);
    static const char* signalName(int sig);
    
private:
    // Save original signal handlers
    struct sigaction mOldHandlers[32] = {};
    
    // Alternate stack (for stack overflow handling)
    static constexpr size_t kAltStackSize = 64 * 1024;
    char mAltStack[kAltStackSize] = {};
    
    // Static pointer for signal handler to access current instance
    static PosixCrashHandler* sInstance;
};

} // namespace ucf::service

#endif // __APPLE__ || __linux__
