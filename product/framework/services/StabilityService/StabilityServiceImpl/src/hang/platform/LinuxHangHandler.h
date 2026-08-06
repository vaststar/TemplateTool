#pragma once

#include "../IPlatformHangHandler.h"

#if defined(__linux__)

#include <pthread.h>
#include <atomic>
#include <string>

namespace ucf::service {

class LinuxHangHandler : public IPlatformHangHandler
{
public:
    LinuxHangHandler();
    ~LinuxHangHandler() override;
    
    std::string captureMainThreadStack(std::thread::id mainThreadId) const override;
    std::string captureCurrentThreadStack(int skipFrames = 0) const override;
    [[nodiscard]] bool isMainThreadCaptureSupported() const override;

private:
    std::string demangleSymbol(const char* symbol) const;
    
    void installSignalHandler();
    void uninstallSignalHandler();
    std::string requestMainThreadStack() const;
    
    static void signalHandler(int sig);
    
    // Static members for signal handler communication (Linux only)
    static std::atomic<bool> sStackCaptureRequested;
    static std::atomic<bool> sStackCaptureComplete;
    static std::string sCapturedStack;
    static pthread_t sMainThreadPthread;
    
    bool mSignalInstalled{false};
};

} // namespace ucf::service

#endif // defined(__linux__)
