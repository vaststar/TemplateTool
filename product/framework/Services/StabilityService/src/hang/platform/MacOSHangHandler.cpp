#if defined(__APPLE__)

#include "MacOSHangHandler.h"
#include "../../StabilityServiceLogger.h"

#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <sstream>
#include <cstring>

#include <mach/mach.h>
#include <mach/thread_act.h>
#include <mach/mach_init.h>

namespace ucf::service {

MacOSHangHandler::MacOSHangHandler()
    : mMainThreadPthread(pthread_self())
{
    CRASHHANDLER_LOG_DEBUG("MacOSHangHandler created, main thread: " << (void*)mMainThreadPthread);
}

std::string MacOSHangHandler::captureMachThreadStack(pthread_t targetThread) const
{
    mach_port_t machThread = pthread_mach_thread_np(targetThread);
    if (machThread == MACH_PORT_NULL)
    {
        return "[Failed to get mach thread port]";
    }
    
    kern_return_t kr = thread_suspend(machThread);
    if (kr != KERN_SUCCESS)
    {
        return "[Failed to suspend thread: " + std::to_string(kr) + "]";
    }
    
#if defined(__arm64__) || defined(__aarch64__)
    arm_thread_state64_t state;
    mach_msg_type_number_t count = ARM_THREAD_STATE64_COUNT;
    kr = thread_get_state(machThread, ARM_THREAD_STATE64, 
                          reinterpret_cast<thread_state_t>(&state), &count);
#else
    x86_thread_state64_t state;
    mach_msg_type_number_t count = x86_THREAD_STATE64_COUNT;
    kr = thread_get_state(machThread, x86_THREAD_STATE64,
                          reinterpret_cast<thread_state_t>(&state), &count);
#endif
    
    std::ostringstream oss;
    
    if (kr == KERN_SUCCESS)
    {
#if defined(__arm64__) || defined(__aarch64__)
        uint64_t fp = state.__fp;
        uint64_t pc = state.__pc;
#else
        uint64_t fp = state.__rbp;
        uint64_t pc = state.__rip;
#endif
        
        constexpr int kMaxFrames = 128;
        void* frames[kMaxFrames];
        int frameCount = 0;
        
        frames[frameCount++] = reinterpret_cast<void*>(pc);
        
        while (fp != 0 && frameCount < kMaxFrames)
        {
            uint64_t* framePtr = reinterpret_cast<uint64_t*>(fp);
            
            vm_size_t size = sizeof(uint64_t) * 2;
            vm_region_basic_info_data_64_t info;
            mach_msg_type_number_t infoCount = VM_REGION_BASIC_INFO_COUNT_64;
            mach_port_t objectName;
            vm_address_t address = fp;
            
            kr = vm_region_64(mach_task_self(), &address, &size, VM_REGION_BASIC_INFO_64,
                             reinterpret_cast<vm_region_info_t>(&info), &infoCount, &objectName);
            
            if (kr != KERN_SUCCESS || address > fp)
            {
                break;
            }
            
            uint64_t nextFp = framePtr[0];
            uint64_t returnAddr = framePtr[1];
            
            if (returnAddr == 0)
            {
                break;
            }
            
            frames[frameCount++] = reinterpret_cast<void*>(returnAddr);
            
            if (nextFp <= fp)
            {
                break;
            }
            fp = nextFp;
        }
        
        char** symbols = backtrace_symbols(frames, frameCount);
        
        oss << "Main Thread Stack Trace (" << frameCount << " frames):\n";
        
        if (symbols)
        {
            for (int i = 0; i < frameCount; ++i)
            {
                oss << "  #" << i << ": " << demangleSymbol(symbols[i]) << "\n";
            }
            free(symbols);
        }
        else
        {
            for (int i = 0; i < frameCount; ++i)
            {
                Dl_info dlinfo;
                if (dladdr(frames[i], &dlinfo) && dlinfo.dli_sname)
                {
                    oss << "  #" << i << ": " << dlinfo.dli_fname << " " << dlinfo.dli_sname << "\n";
                }
                else
                {
                    oss << "  #" << i << ": " << frames[i] << "\n";
                }
            }
        }
    }
    else
    {
        oss << "[Failed to get thread state: " << kr << "]";
    }
    
    thread_resume(machThread);
    
    return oss.str();
}

std::string MacOSHangHandler::captureMainThreadStack(std::thread::id mainThreadId) const
{
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
