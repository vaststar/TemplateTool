#if defined(__linux__)

#include "LinuxHangHandler.h"
#include "../../StabilityServiceLogger.h"

#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <sstream>
#include <cstring>
#include <signal.h>
#include <chrono>
#include <thread>

namespace ucf::service {

// Static member definitions
std::atomic<bool> LinuxHangHandler::sStackCaptureRequested{false};
std::atomic<bool> LinuxHangHandler::sStackCaptureComplete{false};
std::string LinuxHangHandler::sCapturedStack;
pthread_t LinuxHangHandler::sMainThreadPthread{0};

#define STACK_CAPTURE_SIGNAL SIGUSR1

LinuxHangHandler::LinuxHangHandler()
{
    sMainThreadPthread = pthread_self();
    CRASHHANDLER_LOG_DEBUG("LinuxHangHandler created, main thread: " << (void*)sMainThreadPthread);
    installSignalHandler();
}

LinuxHangHandler::~LinuxHangHandler()
{
    uninstallSignalHandler();
}

void LinuxHangHandler::installSignalHandler()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &LinuxHangHandler::signalHandler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(STACK_CAPTURE_SIGNAL, &sa, nullptr) == 0)
    {
        mSignalInstalled = true;
        CRASHHANDLER_LOG_DEBUG("Signal handler installed for SIGUSR1");
    }
    else
    {
        CRASHHANDLER_LOG_ERROR("Failed to install signal handler: " << strerror(errno));
    }
}

void LinuxHangHandler::uninstallSignalHandler()
{
    if (mSignalInstalled)
    {
        signal(STACK_CAPTURE_SIGNAL, SIG_DFL);
        mSignalInstalled = false;
        CRASHHANDLER_LOG_DEBUG("Signal handler uninstalled");
    }
}

void LinuxHangHandler::signalHandler(int sig)
{
    if (sig != STACK_CAPTURE_SIGNAL || !sStackCaptureRequested.load())
    {
        return;
    }
    
    constexpr int kMaxFrames = 128;
    static void* callstack[kMaxFrames];
    int frames = backtrace(callstack, kMaxFrames);
    
    if (frames > 0)
    {
        char** symbols = backtrace_symbols(callstack, frames);
        if (symbols)
        {
            sCapturedStack.clear();
            sCapturedStack.reserve(4096);
            sCapturedStack = "Main Thread Stack Trace (";
            sCapturedStack += std::to_string(frames);
            sCapturedStack += " frames):\n";
            
            for (int i = 0; i < frames; ++i)
            {
                sCapturedStack += "  #";
                sCapturedStack += std::to_string(i);
                sCapturedStack += ": ";
                sCapturedStack += symbols[i];
                sCapturedStack += "\n";
            }
            
            free(symbols);
        }
        else
        {
            sCapturedStack = "[Failed to get symbols in signal handler]";
        }
    }
    else
    {
        sCapturedStack = "[Failed to capture stack in signal handler]";
    }
    
    sStackCaptureComplete.store(true);
}

std::string LinuxHangHandler::requestMainThreadStack() const
{
    if (!mSignalInstalled || sMainThreadPthread == 0)
    {
        return "[Signal handler not installed or main thread not registered]";
    }
    
    sCapturedStack.clear();
    sStackCaptureComplete.store(false);
    sStackCaptureRequested.store(true);
    
    int result = pthread_kill(sMainThreadPthread, STACK_CAPTURE_SIGNAL);
    if (result != 0)
    {
        sStackCaptureRequested.store(false);
        return "[Failed to send signal to main thread: " + std::string(strerror(result)) + "]";
    }
    
    auto start = std::chrono::steady_clock::now();
    constexpr auto timeout = std::chrono::milliseconds(500);
    
    while (!sStackCaptureComplete.load())
    {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > timeout)
        {
            sStackCaptureRequested.store(false);
            return "[Timeout waiting for main thread stack capture - thread may be completely blocked]";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    sStackCaptureRequested.store(false);
    return sCapturedStack;
}

std::string LinuxHangHandler::captureMainThreadStack(std::thread::id mainThreadId) const
{
    std::ostringstream oss;
    
    oss << "[Hang Detected - Main Thread Stack Capture]\n";
    oss << "Platform: Linux\n\n";
    
    if (mSignalInstalled)
    {
        oss << requestMainThreadStack();
    }
    else
    {
        oss << "[Signal-based capture not available]\n";
        oss << "[Capturing watchdog thread stack as reference]\n\n";
        oss << captureCurrentThreadStack(1);
    }
    
    return oss.str();
}

std::string LinuxHangHandler::captureCurrentThreadStack(int skipFrames) const
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
        oss << "  #" << (i - skipFrames) << ": " << demangleSymbol(symbols[i]) << "\n";
    }
    
    free(symbols);
    return oss.str();
}

bool LinuxHangHandler::isMainThreadCaptureSupported() const
{
    return mSignalInstalled && sMainThreadPthread != 0;
}

std::string LinuxHangHandler::demangleSymbol(const char* symbol) const
{
    std::string result(symbol);
    
    const char* mangled = strstr(symbol, "_Z");
    if (!mangled)
    {
        return result;
    }
    
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

#endif // defined(__linux__)
