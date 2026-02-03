#if defined(__APPLE__) || defined(__linux__)

#include "PosixCrashHandler.h"
#include "../../StabilityServiceLogger.h"

#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <cstring>
#include <sstream>

namespace ucf::service {

// Signals to capture
static constexpr int kCrashSignals[] = {
    SIGSEGV,  // Segmentation fault (null pointer, illegal memory access)
    SIGABRT,  // abort() or assert failure
    SIGBUS,   // Bus error
    SIGFPE,   // Floating point exception (division by zero)
    SIGILL,   // Illegal instruction
    SIGTRAP,  // Breakpoint/debug trap (UB in Release mode may trigger)
};

// 静态成员初始化
PosixCrashHandler* PosixCrashHandler::sInstance = nullptr;

PosixCrashHandler::PosixCrashHandler(Config config)
    : IPlatformCrashHandler(std::move(config))
{
    install();
}

PosixCrashHandler::~PosixCrashHandler()
{
    uninstall();
}

void PosixCrashHandler::doInstall()
{
    sInstance = this;
    
    // Set alternate stack (for stack overflow)
    stack_t ss;
    ss.ss_sp = mAltStack;
    ss.ss_size = kAltStackSize;
    ss.ss_flags = 0;
    if (sigaltstack(&ss, nullptr) != 0)
    {
        CRASHHANDLER_LOG_WARN("Failed to set alternate stack");
    }
    
    // Install signal handlers
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_sigaction = signalHandler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    
    for (int sig : kCrashSignals)
    {
        if (sigaction(sig, &action, &mOldHandlers[sig]) != 0)
        {
            CRASHHANDLER_LOG_ERROR("Failed to install handler for signal " << sig);
        }
    }
    
    CRASHHANDLER_LOG_INFO("PosixCrashHandler installed");
}

void PosixCrashHandler::doUninstall()
{
    // Restore original signal handlers
    for (int sig : kCrashSignals)
    {
        sigaction(sig, &mOldHandlers[sig], nullptr);
    }
    
    sInstance = nullptr;
    mCallback = nullptr;
    
    CRASHHANDLER_LOG_INFO("PosixCrashHandler uninstalled");
}

void PosixCrashHandler::signalHandler(int sig, siginfo_t* info, void* context)
{
    // Prevent re-entry (crash during crash handling)
    static volatile sig_atomic_t sCrashing = 0;
    if (sCrashing)
    {
        _exit(1);
    }
    sCrashing = 1;
    
    // Notify via callback to handle crash
    if (sInstance && sInstance->mCallback)
    {
        sInstance->mCallback(sig, signalName(sig));
    }
    
    // Restore default handler and re-raise signal
    // This allows core dump generation (if system permits)
    signal(sig, SIG_DFL);
    raise(sig);
}

const char* PosixCrashHandler::signalName(int sig)
{
    switch (sig)
    {
        case SIGSEGV: return "SIGSEGV (Segmentation Fault)";
        case SIGABRT: return "SIGABRT (Abort)";
        case SIGBUS:  return "SIGBUS (Bus Error)";
        case SIGFPE:  return "SIGFPE (Floating Point Exception)";
        case SIGILL:  return "SIGILL (Illegal Instruction)";
        case SIGTRAP: return "SIGTRAP (Breakpoint/Trap)";
        default:      return "Unknown Signal";
    }
}

std::string PosixCrashHandler::captureStackTrace(int skipFrames) const
{
    std::ostringstream oss;
    
    void* callstack[128];
    int frames = backtrace(callstack, 128);
    
    if (frames == 0)
    {
        return "  <empty stack trace>\n";
    }
    
    char** symbols = backtrace_symbols(callstack, frames);
    if (!symbols)
    {
        return "  <failed to get symbols>\n";
    }
    
    for (int i = skipFrames; i < frames; ++i)
    {
        Dl_info info;
        if (dladdr(callstack[i], &info) && info.dli_sname)
        {
            // Try to demangle C++ symbols
            int status = 0;
            char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
            
            oss << "#" << (i - skipFrames) << "  ";
            
            if (status == 0 && demangled)
            {
                oss << demangled;
                free(demangled);
            }
            else
            {
                oss << info.dli_sname;
            }
            
            // Offset
            ptrdiff_t offset = static_cast<char*>(callstack[i]) - static_cast<char*>(info.dli_saddr);
            oss << " + " << offset;
            
            // Module name
            if (info.dli_fname)
            {
                const char* basename = strrchr(info.dli_fname, '/');
                oss << " (" << (basename ? basename + 1 : info.dli_fname) << ")";
            }
            oss << "\n";
        }
        else
        {
            // Fallback to raw symbols
            oss << "#" << (i - skipFrames) << "  " << symbols[i] << "\n";
        }
    }
    
    free(symbols);
    return oss.str();
}

} // namespace ucf::service

#endif // __APPLE__ || __linux__
