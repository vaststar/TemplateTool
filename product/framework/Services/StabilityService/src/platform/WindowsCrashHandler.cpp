#ifdef _WIN32

#include "WindowsCrashHandler.h"
#include "../StabilityServiceLogger.h"

#include <DbgHelp.h>
#include <sstream>

#pragma comment(lib, "DbgHelp.lib")

namespace ucf::service {

// 静态成员初始化
WindowsCrashHandler* WindowsCrashHandler::sInstance = nullptr;

WindowsCrashHandler::WindowsCrashHandler(Config config)
    : IPlatformCrashHandler(std::move(config))
{
    install();
}

WindowsCrashHandler::~WindowsCrashHandler()
{
    uninstall();
}

void WindowsCrashHandler::doInstall()
{
    sInstance = this;
    
    mOldFilter = SetUnhandledExceptionFilter(exceptionFilter);
    
    CRASHHANDLER_LOG_INFO("WindowsCrashHandler installed");
}

void WindowsCrashHandler::doUninstall()
{
    SetUnhandledExceptionFilter(mOldFilter);
    
    sInstance = nullptr;
    
    CRASHHANDLER_LOG_INFO("WindowsCrashHandler uninstalled");
}

LONG WINAPI WindowsCrashHandler::exceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
    if (sInstance && sInstance->mCallback)
    {
        DWORD code = exceptionInfo->ExceptionRecord->ExceptionCode;
        
        // Call callback first to write crash log
        sInstance->mCallback(static_cast<int>(code), exceptionName(code));
        
        // Write minidump
        if (!sInstance->mCrashDir.empty())
        {
            writeMinidump(exceptionInfo, sInstance->mCrashDir);
        }
    }
    
    return EXCEPTION_EXECUTE_HANDLER;
}

const char* WindowsCrashHandler::exceptionName(DWORD code)
{
    switch (code)
    {
        case EXCEPTION_ACCESS_VIOLATION:      return "EXCEPTION_ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT:            return "EXCEPTION_BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DENORMAL_OPERAND:  return "EXCEPTION_FLT_DENORMAL_OPERAND";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:    return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_INEXACT_RESULT:    return "EXCEPTION_FLT_INEXACT_RESULT";
        case EXCEPTION_FLT_INVALID_OPERATION: return "EXCEPTION_FLT_INVALID_OPERATION";
        case EXCEPTION_FLT_OVERFLOW:          return "EXCEPTION_FLT_OVERFLOW";
        case EXCEPTION_FLT_STACK_CHECK:       return "EXCEPTION_FLT_STACK_CHECK";
        case EXCEPTION_FLT_UNDERFLOW:         return "EXCEPTION_FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION:   return "EXCEPTION_ILLEGAL_INSTRUCTION";
        case EXCEPTION_IN_PAGE_ERROR:         return "EXCEPTION_IN_PAGE_ERROR";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:    return "EXCEPTION_INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW:          return "EXCEPTION_INT_OVERFLOW";
        case EXCEPTION_INVALID_DISPOSITION:   return "EXCEPTION_INVALID_DISPOSITION";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
        case EXCEPTION_PRIV_INSTRUCTION:      return "EXCEPTION_PRIV_INSTRUCTION";
        case EXCEPTION_SINGLE_STEP:           return "EXCEPTION_SINGLE_STEP";
        case EXCEPTION_STACK_OVERFLOW:        return "EXCEPTION_STACK_OVERFLOW";
        default:                              return "Unknown Exception";
    }
}

void WindowsCrashHandler::writeMinidump(EXCEPTION_POINTERS* exceptionInfo, const std::filesystem::path& crashDir)
{
    // Generate timestamped filename
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    wchar_t filename[64];
    swprintf_s(filename, L"crash_%04d%02d%02d_%02d%02d%02d.dmp",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    
    auto path = crashDir / filename;
    
    HANDLE hFile = CreateFileW(
        path.wstring().c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }
    
    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = exceptionInfo;
    mei.ClientPointers = FALSE;
    
    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpWithFullMemory,
        &mei,
        NULL,
        NULL
    );
    
    CloseHandle(hFile);
}

std::string WindowsCrashHandler::captureStackTrace(int skipFrames) const
{
    std::ostringstream oss;
    
    void* stack[128];
    HANDLE process = GetCurrentProcess();
    
    SymInitialize(process, NULL, TRUE);
    
    WORD frames = CaptureStackBackTrace(skipFrames, 128, stack, NULL);
    
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256, 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    
    // For line number info
    IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    DWORD displacement = 0;
    
    for (WORD i = 0; i < frames; i++)
    {
        DWORD64 address = (DWORD64)stack[i];
        
        oss << "#" << std::dec << i << "  ";
        
        // Try to get symbol name
        if (SymFromAddr(process, address, 0, symbol) && symbol->Name[0] != '\0')
        {
            oss << symbol->Name;
            
            // Try to get line number (requires PDB)
            if (SymGetLineFromAddr64(process, address, &displacement, &line))
            {
                oss << " at " << line.FileName << ":" << line.LineNumber;
            }
        }
        else
        {
            // No symbol available, output module + offset instead
            HMODULE hModule = NULL;
            GetModuleHandleExW(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCWSTR)address,
                &hModule
            );
            
            if (hModule)
            {
                wchar_t modulePath[MAX_PATH];
                if (GetModuleFileNameW(hModule, modulePath, MAX_PATH))
                {
                    // Extract just the filename
                    const wchar_t* moduleName = wcsrchr(modulePath, L'\\');
                    moduleName = moduleName ? moduleName + 1 : modulePath;
                    
                    // Calculate offset from module base
                    DWORD64 moduleBase = (DWORD64)hModule;
                    DWORD64 offset = address - moduleBase;
                    
                    // Convert wide string to narrow for output
                    char narrowName[MAX_PATH];
                    WideCharToMultiByte(CP_UTF8, 0, moduleName, -1, narrowName, MAX_PATH, NULL, NULL);
                    
                    oss << narrowName << "+0x" << std::hex << offset;
                }
                else
                {
                    oss << "0x" << std::hex << address;
                }
            }
            else
            {
                oss << "0x" << std::hex << address;
            }
        }
        
        oss << "\n";
    }
    
    free(symbol);
    SymCleanup(process);
    
    return oss.str();
}

} // namespace ucf::service

#endif // _WIN32
