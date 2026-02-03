#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace ucf::service {

/// Abstract interface for platform hang handlers
/// Responsible for capturing stack traces of other threads
class IPlatformHangHandler
{
public:
    virtual ~IPlatformHangHandler() = default;
    
    /// Factory method: create platform hang handler
    /// @return Platform-specific hang handler
    static std::unique_ptr<IPlatformHangHandler> create();
    
    /// Capture main thread stack trace
    /// This is called from watchdog thread to capture the main/UI thread's stack
    /// @param mainThreadId The thread ID of the main thread to capture
    /// @return Stack trace string, or error message if capture failed
    virtual std::string captureMainThreadStack(std::thread::id mainThreadId) const = 0;
    
    /// Capture current thread stack trace (for reference/debugging)
    /// @param skipFrames Number of stack frames to skip
    /// @return Stack trace string
    virtual std::string captureCurrentThreadStack(int skipFrames = 0) const = 0;
    
    /// Check if main thread stack capture is supported on this platform
    [[nodiscard]] virtual bool isMainThreadCaptureSupported() const = 0;
    
protected:
    IPlatformHangHandler() = default;
    
    // Disable copy
    IPlatformHangHandler(const IPlatformHangHandler&) = delete;
    IPlatformHangHandler& operator=(const IPlatformHangHandler&) = delete;
    IPlatformHangHandler(IPlatformHangHandler&&) = delete;
    IPlatformHangHandler& operator=(IPlatformHangHandler&&) = delete;
};

} // namespace ucf::service
