#ifdef _WIN32

#include "WindowsCrashHandler.h"
#include "../CrashHandlerServiceLogger.h"

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
    auto path = crashDir / "crash.dmp";
    
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
    
    for (WORD i = 0; i < frames; i++)
    {
        SymFromAddr(process, (DWORD64)stack[i], 0, symbol);
        oss << "#" << i << "  " << symbol->Name << " - 0x" << std::hex << symbol->Address << "\n";
    }
    
    free(symbol);
    
    return oss.str();
}

} // namespace ucf::service

#endif // _WIN32
