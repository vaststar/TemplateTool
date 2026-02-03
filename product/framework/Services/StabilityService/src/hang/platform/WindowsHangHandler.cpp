#if defined(_WIN32)

#include "WindowsHangHandler.h"
#include "../../StabilityServiceLogger.h"

#include <windows.h>
#include <tlhelp32.h>
#include <dbghelp.h>
#include <sstream>

#pragma comment(lib, "dbghelp.lib")

namespace ucf::service {

WindowsHangHandler::WindowsHangHandler()
{
    // Initialize symbol handler for the process
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
    
    CRASHHANDLER_LOG_DEBUG("WindowsHangHandler created");
}

std::string WindowsHangHandler::captureMainThreadStack(std::thread::id mainThreadId) const
{
    std::ostringstream oss;
    
    // Get the main thread handle
    // We need to enumerate threads to find the main thread
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return "[Failed to create thread snapshot]";
    }
    
    DWORD currentProcessId = GetCurrentProcessId();
    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);
    
    HANDLE hMainThread = NULL;
    
    if (Thread32First(hSnapshot, &te32))
    {
        do
        {
            if (te32.th32OwnerProcessID == currentProcessId)
            {
                // Try to open each thread and check if it matches
                HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,
                                           FALSE, te32.th32ThreadID);
                if (hThread)
                {
                    // Compare thread IDs
                    // Note: std::thread::id comparison with native ID is platform-specific
                    // For now, we'll capture the first thread (usually the main thread)
                    if (!hMainThread)
                    {
                        hMainThread = hThread;
                        break;
                    }
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hSnapshot, &te32));
    }
    
    CloseHandle(hSnapshot);
    
    if (!hMainThread)
    {
        return "[Failed to find main thread]";
    }
    
    oss << "[Hang Detected - Main Thread Stack]\n\n";
    oss << captureThreadStack(hMainThread, 0);
    
    CloseHandle(hMainThread);
    return oss.str();
}

std::string WindowsHangHandler::captureCurrentThreadStack(int skipFrames) const
{
    return captureThreadStack(GetCurrentThread(), skipFrames + 1);  // +1 for this function
}

std::string WindowsHangHandler::captureThreadStack(void* threadHandle, int skipFrames) const
{
    HANDLE hThread = static_cast<HANDLE>(threadHandle);
    HANDLE hProcess = GetCurrentProcess();
    
    // Suspend the thread to capture its context
    bool needResume = (hThread != GetCurrentThread());
    if (needResume)
    {
        SuspendThread(hThread);
    }
    
    CONTEXT context;
    memset(&context, 0, sizeof(context));
    context.ContextFlags = CONTEXT_FULL;
    
    if (!GetThreadContext(hThread, &context))
    {
        if (needResume)
        {
            ResumeThread(hThread);
        }
        return "[Failed to get thread context]";
    }
    
    STACKFRAME64 stackFrame;
    memset(&stackFrame, 0, sizeof(stackFrame));
    
#if defined(_M_X64)
    DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
    stackFrame.AddrPC.Offset = context.Rip;
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context.Rbp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context.Rsp;
    stackFrame.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_IX86)
    DWORD machineType = IMAGE_FILE_MACHINE_I386;
    stackFrame.AddrPC.Offset = context.Eip;
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context.Ebp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context.Esp;
    stackFrame.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_ARM64)
    DWORD machineType = IMAGE_FILE_MACHINE_ARM64;
    stackFrame.AddrPC.Offset = context.Pc;
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context.Fp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context.Sp;
    stackFrame.AddrStack.Mode = AddrModeFlat;
#else
    #error "Unsupported architecture"
#endif

    std::ostringstream oss;
    int frameNumber = 0;
    
    constexpr int kMaxFrames = 128;
    
    while (frameNumber < kMaxFrames)
    {
        if (!StackWalk64(machineType, hProcess, hThread, &stackFrame, &context,
                        NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
        {
            break;
        }
        
        if (stackFrame.AddrPC.Offset == 0)
        {
            break;
        }
        
        if (frameNumber >= skipFrames)
        {
            // Get symbol information
            char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;
            pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            pSymbol->MaxNameLen = MAX_SYM_NAME;
            
            DWORD64 displacement = 0;
            
            if (SymFromAddr(hProcess, stackFrame.AddrPC.Offset, &displacement, pSymbol))
            {
                oss << "  #" << (frameNumber - skipFrames) << ": " << pSymbol->Name;
                
                // Try to get line information
                IMAGEHLP_LINE64 line;
                line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                DWORD lineDisplacement = 0;
                
                if (SymGetLineFromAddr64(hProcess, stackFrame.AddrPC.Offset, &lineDisplacement, &line))
                {
                    oss << " at " << line.FileName << ":" << line.LineNumber;
                }
                
                oss << " + 0x" << std::hex << displacement << std::dec << "\n";
            }
            else
            {
                oss << "  #" << (frameNumber - skipFrames) << ": [0x" 
                    << std::hex << stackFrame.AddrPC.Offset << std::dec << "]\n";
            }
        }
        
        ++frameNumber;
    }
    
    if (needResume)
    {
        ResumeThread(hThread);
    }
    
    if (oss.str().empty())
    {
        return "[No stack frames captured]";
    }
    
    return "Stack trace (" + std::to_string(frameNumber - skipFrames) + " frames):\n" + oss.str();
}

bool WindowsHangHandler::isMainThreadCaptureSupported() const
{
    return true;  // Windows supports capturing other thread stacks
}

std::string WindowsHangHandler::demangleSymbol(const char* symbol) const
{
    // SymFromAddr already returns demangled names on Windows when SYMOPT_UNDNAME is set
    return std::string(symbol);
}

} // namespace ucf::service

#endif // defined(_WIN32)
