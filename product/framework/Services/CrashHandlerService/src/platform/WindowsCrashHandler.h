#pragma once

#ifdef _WIN32

#include "IPlatformCrashHandler.h"

#include <Windows.h>

namespace ucf::service {

class WindowsCrashHandler final : public IPlatformCrashHandler
{
public:
    /// Construct and install exception filter
    explicit WindowsCrashHandler(Config config);
    
    /// Auto-uninstall on destruction
    ~WindowsCrashHandler() override;
    
    // IPlatformCrashHandler interface implementation
    std::string captureStackTrace(int skipFrames = 0) const override;
    
protected:
    // Base class pure virtual function implementation
    void doInstall() override;
    void doUninstall() override;
    
private:
    static LONG WINAPI exceptionFilter(EXCEPTION_POINTERS* exceptionInfo);
    static const char* exceptionName(DWORD code);
    static void writeMinidump(EXCEPTION_POINTERS* exceptionInfo, const std::filesystem::path& crashDir);
    
private:
    LPTOP_LEVEL_EXCEPTION_FILTER mOldFilter = nullptr;
    
    // Static pointer for exception filter to access current instance
    static WindowsCrashHandler* sInstance;
};

} // namespace ucf::service

#endif // _WIN32
