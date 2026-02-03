#if defined(__APPLE__) || defined(__linux__)

#include "PosixHangHandler.h"
#include "../../StabilityServiceLogger.h"

#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <sstream>
#include <cstring>

#if defined(__APPLE__)
#include <pthread.h>
#include <mach/mach.h>
#include <mach/thread_act.h>
#endif

namespace ucf::service {

PosixHangHandler::PosixHangHandler()
{
    CRASHHANDLER_LOG_DEBUG("PosixHangHandler created");
}

std::string PosixHangHandler::captureMainThreadStack(std::thread::id mainThreadId) const
{
    std::ostringstream oss;
    
    // Note: Capturing another thread's stack trace on POSIX is complex
    // On macOS, we could use thread_get_state + thread_suspend, but this requires
    // converting std::thread::id to mach_port_t which is not straightforward
    
    // For now, we provide a placeholder with useful diagnostic info
    oss << "[Hang Detected - Main Thread Stack Capture]\n";
    oss << "Platform: ";
#if defined(__APPLE__)
    oss << "macOS\n";
#else
    oss << "Linux\n";
#endif
    
    // Try to get main thread info on macOS
#if defined(__APPLE__)
    // Get the main thread's mach port
    // Note: This is a simplified approach - full implementation would need
    // to enumerate threads and match by thread ID
    oss << "\n[Note: Full main thread stack capture requires thread enumeration]\n";
    oss << "[Capturing watchdog thread stack as reference]\n\n";
#else
    oss << "\n[Note: Linux main thread capture requires ptrace or /proc/self/task]\n";
    oss << "[Capturing watchdog thread stack as reference]\n\n";
#endif
    
    // Capture current thread's stack as a fallback/reference
    oss << captureCurrentThreadStack(1);  // Skip this function
    
    return oss.str();
}

std::string PosixHangHandler::captureCurrentThreadStack(int skipFrames) const
{
    constexpr int kMaxFrames = 128;
    void* callstack[kMaxFrames];
    
    int frames = backtrace(callstack, kMaxFrames);
    if (frames <= 0)
    {
        return "[Failed to capture stack trace]";
    }
    
    char** symbols = backtrace_symbols(callstack, frames);
    if (!symbols)
    {
        return "[Failed to get symbols]";
    }
    
    std::ostringstream oss;
    oss << "Stack trace (" << (frames - skipFrames) << " frames):\n";
    
    for (int i = skipFrames; i < frames; ++i)
    {
        std::string demangled = demangleSymbol(symbols[i]);
        oss << "  #" << (i - skipFrames) << ": " << demangled << "\n";
    }
    
    free(symbols);
    return oss.str();
}

bool PosixHangHandler::isMainThreadCaptureSupported() const
{
    // Full main thread capture is not fully implemented yet
    // We return false to indicate that captureMainThreadStack provides limited info
    return false;
}

std::string PosixHangHandler::demangleSymbol(const char* symbol) const
{
    // Try to extract and demangle the function name from backtrace_symbols format
    // Format on macOS: "1   libFoo.dylib  0x00007fff... _ZN3Foo3barEv + 42"
    // Format on Linux: "./program(_ZN3Foo3barEv+0x42) [0x...]"
    
    std::string result(symbol);
    
    // Find the mangled name (starts with _Z for C++ symbols)
    const char* mangled = strstr(symbol, "_Z");
    if (!mangled)
    {
        return result;  // No C++ mangling found
    }
    
    // Find the end of the mangled name
    const char* end = mangled;
    while (*end && *end != ' ' && *end != '+' && *end != ')' && *end != ']')
    {
        ++end;
    }
    
    std::string mangledName(mangled, end - mangled);
    
    int status = 0;
    char* demangled = abi::__cxa_demangle(mangledName.c_str(), nullptr, nullptr, &status);
    
    if (status == 0 && demangled)
    {
        // Replace mangled name with demangled name in result
        size_t pos = result.find(mangledName);
        if (pos != std::string::npos)
        {
            result.replace(pos, mangledName.length(), demangled);
        }
        free(demangled);
    }
    
    return result;
}

} // namespace ucf::service

#endif // defined(__APPLE__) || defined(__linux__)
