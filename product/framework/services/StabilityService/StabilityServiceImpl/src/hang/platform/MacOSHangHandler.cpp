#if defined(__APPLE__)

#include "MacOSHangHandler.h"
#include "../../StabilityServiceLogger.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <sstream>
#include <cstring>

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/thread_act.h>
#include <mach/mach_init.h>

namespace ucf::service {

namespace {

constexpr std::size_t kMaxCapturedFrames = 128;

struct RawStackCapture
{
    std::array<void*, kMaxCapturedFrames> frames{};
    int frameCount{0};
    kern_return_t threadStateResult{KERN_FAILURE};
    kern_return_t resumeResult{KERN_FAILURE};
};

struct StackFrameRecord
{
    mach_vm_address_t previousFramePointer{0};
    mach_vm_address_t returnAddress{0};
};

class ThreadResumeGuard final
{
public:
    ThreadResumeGuard(thread_act_t thread, kern_return_t& resumeResult) noexcept
        : mThread(thread)
        , mResumeResult(resumeResult)
    {
    }

    ThreadResumeGuard(const ThreadResumeGuard&) = delete;
    ThreadResumeGuard& operator=(const ThreadResumeGuard&) = delete;

    ~ThreadResumeGuard() noexcept
    {
        mResumeResult = thread_resume(mThread);
    }

private:
    thread_act_t mThread;
    kern_return_t& mResumeResult;
};

} // namespace

MacOSHangHandler::MacOSHangHandler()
    : mMainThreadPthread(pthread_self())
{
    CRASHHANDLER_LOG_DEBUG("MacOSHangHandler created, main thread: " << (void*)mMainThreadPthread);
}

std::string MacOSHangHandler::captureMachThreadStack(pthread_t targetThread) const
{
    const mach_port_t machThread = pthread_mach_thread_np(targetThread);
    if (machThread == MACH_PORT_NULL)
    {
        return "[Failed to get mach thread port]";
    }

    const kern_return_t suspendResult = thread_suspend(machThread);
    if (suspendResult != KERN_SUCCESS)
    {
        return "[Failed to suspend thread: " + std::to_string(suspendResult) + "]";
    }

    RawStackCapture capture;

    // While the main thread is suspended, do not log, allocate memory,
    // construct strings or perform symbol lookup.
    {
        ThreadResumeGuard resumeGuard(machThread, capture.resumeResult);

#if defined(__arm64__) || defined(__aarch64__)
        arm_thread_state64_t state{};
        mach_msg_type_number_t stateCount = ARM_THREAD_STATE64_COUNT;
        capture.threadStateResult = thread_get_state(machThread, ARM_THREAD_STATE64, reinterpret_cast<thread_state_t>(&state), &stateCount);
#else
        x86_thread_state64_t state{};
        mach_msg_type_number_t stateCount = x86_THREAD_STATE64_COUNT;
        capture.threadStateResult = thread_get_state(machThread, x86_THREAD_STATE64, reinterpret_cast<thread_state_t>(&state), &stateCount);
#endif

        if (capture.threadStateResult == KERN_SUCCESS)
        {
#if defined(__arm64__) || defined(__aarch64__)
            mach_vm_address_t framePointer = state.__fp;
            const mach_vm_address_t programCounter = state.__pc;
#else
            mach_vm_address_t framePointer = state.__rbp;
            const mach_vm_address_t programCounter = state.__rip;
#endif

            if (programCounter != 0)
            {
                capture.frames[capture.frameCount++] = reinterpret_cast<void*>(programCounter);
            }

            while (framePointer != 0 && capture.frameCount < static_cast<int>(capture.frames.size()))
            {
                if (framePointer % alignof(StackFrameRecord) != 0)
                {
                    break;
                }

                StackFrameRecord frame{};
                mach_vm_size_t bytesRead{0};
                const kern_return_t readResult = mach_vm_read_overwrite(mach_task_self(), framePointer, sizeof(frame), reinterpret_cast<mach_vm_address_t>(&frame), &bytesRead);
                if (readResult != KERN_SUCCESS || bytesRead != sizeof(frame) || frame.returnAddress == 0)
                {
                    break;
                }

                capture.frames[capture.frameCount++] = reinterpret_cast<void*>(frame.returnAddress);
                if (frame.previousFramePointer <= framePointer)
                {
                    break;
                }

                framePointer = frame.previousFramePointer;
            }
        }
    }

    if (capture.resumeResult != KERN_SUCCESS)
    {
        return "[Failed to resume main thread: " + std::to_string(capture.resumeResult) + "]";
    }

    CRASHHANDLER_LOG_DEBUG("macOS main thread resumed after raw stack capture, frameCount: " << capture.frameCount);

    std::ostringstream oss;
    if (capture.threadStateResult != KERN_SUCCESS)
    {
        oss << "[Failed to get thread state: " << capture.threadStateResult << "]";
        return oss.str();
    }

    oss << "Main Thread Stack Trace (" << capture.frameCount << " frames):\n";

    // Symbol resolution may allocate memory or acquire dyld locks, so it must
    // run only after the main thread has resumed.
    char** symbols = backtrace_symbols(capture.frames.data(), capture.frameCount);
    if (symbols)
    {
        for (int i = 0; i < capture.frameCount; ++i)
        {
            oss << "  #" << i << ": " << demangleSymbol(symbols[i]) << "\n";
        }
        free(symbols);
    }
    else
    {
        for (int i = 0; i < capture.frameCount; ++i)
        {
            Dl_info dlinfo{};
            if (dladdr(capture.frames[i], &dlinfo) && dlinfo.dli_sname)
            {
                oss << "  #" << i << ": " << dlinfo.dli_fname << " " << dlinfo.dli_sname << "\n";
            }
            else
            {
                oss << "  #" << i << ": " << capture.frames[i] << "\n";
            }
        }
    }

    return oss.str();
}

std::string MacOSHangHandler::captureMainThreadStack(std::thread::id mainThreadId) const
{
    static_cast<void>(mainThreadId);
    std::ostringstream oss;
    
    oss << "[Hang Detected - Main Thread Stack Capture]\n";
    oss << "Platform: macOS\n\n";
    
    if (mMainThreadPthread != 0)
    {
        oss << captureMachThreadStack(mMainThreadPthread);
    }
    else
    {
        oss << "[Main thread not registered]\n";
    }
    
    return oss.str();
}

std::string MacOSHangHandler::captureCurrentThreadStack(int skipFrames) const
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

bool MacOSHangHandler::isMainThreadCaptureSupported() const
{
    return mMainThreadPthread != 0;
}

std::string MacOSHangHandler::demangleSymbol(const char* symbol) const
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

#endif // defined(__APPLE__)
