#ifdef _WIN32

#include "ProcessLauncher.h"

#include <sstream>

namespace ucf::utilities::detail {

ProcessLauncher::ProcessHandle ProcessLauncher::launch(
    const std::string& executable,
    const std::vector<std::string>& args,
    const std::string& workingDir,
    bool captureStdout,
    bool captureStderr)
{
    ProcessHandle handle;

    // ── Build command line ──
    // Quote the executable and each argument
    std::ostringstream cmdLine;
    cmdLine << "\"" << executable << "\"";
    for (const auto& arg : args)
    {
        cmdLine << " \"" << arg << "\"";
    }
    std::string cmdStr = cmdLine.str();

    // CreateProcessW needs a mutable wide string
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, cmdStr.c_str(), -1, nullptr, 0);
    std::wstring wideCmdLine(static_cast<size_t>(wideLen), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, cmdStr.c_str(), -1, wideCmdLine.data(), wideLen);

    // Working directory (wide)
    std::wstring wideWorkDir;
    const wchar_t* pWorkDir = nullptr;
    if (!workingDir.empty())
    {
        int dirLen = MultiByteToWideChar(CP_UTF8, 0, workingDir.c_str(), -1, nullptr, 0);
        wideWorkDir.resize(static_cast<size_t>(dirLen), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, workingDir.c_str(), -1, wideWorkDir.data(), dirLen);
        pWorkDir = wideWorkDir.c_str();
    }

    // ── Create pipes for stdout/stderr ──
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;

    HANDLE hStdoutReadTmp = nullptr, hStdoutWrite = nullptr;
    HANDLE hStderrReadTmp = nullptr, hStderrWrite = nullptr;

    if (captureStdout)
    {
        if (!CreatePipe(&hStdoutReadTmp, &hStdoutWrite, &sa, 0))
        {
            handle.errorMessage = "Failed to create stdout pipe";
            return handle;
        }
        // Ensure the read end is not inherited
        SetHandleInformation(hStdoutReadTmp, HANDLE_FLAG_INHERIT, 0);
    }

    if (captureStderr)
    {
        if (!CreatePipe(&hStderrReadTmp, &hStderrWrite, &sa, 0))
        {
            handle.errorMessage = "Failed to create stderr pipe";
            if (hStdoutReadTmp)
            {
                CloseHandle(hStdoutReadTmp);
                CloseHandle(hStdoutWrite);
            }
            return handle;
        }
        SetHandleInformation(hStderrReadTmp, HANDLE_FLAG_INHERIT, 0);
    }

    // ── Launch process ──
    STARTUPINFOW si{};
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = hStdoutWrite ? hStdoutWrite : GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError  = hStderrWrite ? hStderrWrite : GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi{};
    DWORD creationFlags = CREATE_NO_WINDOW;

    BOOL ok = CreateProcessW(
        nullptr,
        wideCmdLine.data(),
        nullptr, nullptr,
        TRUE,  // inherit handles
        creationFlags,
        nullptr,
        pWorkDir,
        &si,
        &pi);

    // Close the write ends — child has inherited them
    if (hStdoutWrite)
    {
        CloseHandle(hStdoutWrite);
    }
    if (hStderrWrite)
    {
        CloseHandle(hStderrWrite);
    }

    if (!ok)
    {
        DWORD err = GetLastError();
        handle.errorMessage = "CreateProcess failed, error code: " + std::to_string(err);
        if (hStdoutReadTmp)
        {
            CloseHandle(hStdoutReadTmp);
        }
        if (hStderrReadTmp)
        {
            CloseHandle(hStderrReadTmp);
        }
        return handle;
    }

    handle.valid = true;
    handle.pid = static_cast<int64_t>(pi.dwProcessId);
    handle.hProcess = pi.hProcess;
    handle.hThread  = pi.hThread;
    handle.hStdoutRead = hStdoutReadTmp;
    handle.hStderrRead = hStderrReadTmp;
    return handle;
}

bool ProcessLauncher::terminate(const ProcessHandle& handle)
{
    if (!handle.valid || !handle.hProcess)
    {
        return false;
    }
    return TerminateProcess(handle.hProcess, 0) != 0;
}

bool ProcessLauncher::kill(const ProcessHandle& handle)
{
    if (!handle.valid || !handle.hProcess)
    {
        return false;
    }
    return TerminateProcess(handle.hProcess, 1) != 0;
}

bool ProcessLauncher::isAlive(const ProcessHandle& handle)
{
    if (!handle.valid || !handle.hProcess)
    {
        return false;
    }
    DWORD exitCode = 0;
    if (!GetExitCodeProcess(handle.hProcess, &exitCode))
    {
        return false;
    }
    return exitCode == STILL_ACTIVE;
}

int ProcessLauncher::waitForExit(const ProcessHandle& handle, int timeoutMs)
{
    if (!handle.valid || !handle.hProcess)
    {
        return -1;
    }

    DWORD timeout = (timeoutMs < 0) ? INFINITE : static_cast<DWORD>(timeoutMs);
    DWORD result = WaitForSingleObject(handle.hProcess, timeout);

    if (result == WAIT_TIMEOUT)
    {
        return -1;
    }

    DWORD exitCode = 0;
    GetExitCodeProcess(handle.hProcess, &exitCode);
    return static_cast<int>(exitCode);
}

static std::string readPipe(HANDLE hPipe)
{
    if (!hPipe)
    {
        return {};
    }

    // Check if there's data available without blocking
    DWORD available = 0;
    if (!PeekNamedPipe(hPipe, nullptr, 0, nullptr, &available, nullptr) || available == 0)
    {
        return {};
    }

    std::string buffer(static_cast<size_t>(available), '\0');
    DWORD bytesRead = 0;
    if (!ReadFile(hPipe, buffer.data(), available, &bytesRead, nullptr))
    {
        return {};
    }

    buffer.resize(bytesRead);
    return buffer;
}

std::string ProcessLauncher::readStdout(const ProcessHandle& handle)
{
    return readPipe(handle.hStdoutRead);
}

std::string ProcessLauncher::readStderr(const ProcessHandle& handle)
{
    return readPipe(handle.hStderrRead);
}

void ProcessLauncher::closeHandles(ProcessHandle& handle)
{
    if (handle.hStdoutRead)
    {
        CloseHandle(handle.hStdoutRead);
        handle.hStdoutRead = nullptr;
    }
    if (handle.hStderrRead)
    {
        CloseHandle(handle.hStderrRead);
        handle.hStderrRead = nullptr;
    }
    if (handle.hThread)
    {
        CloseHandle(handle.hThread);
        handle.hThread = nullptr;
    }
    if (handle.hProcess)
    {
        CloseHandle(handle.hProcess);
        handle.hProcess = nullptr;
    }
    handle.valid = false;
}

} // namespace ucf::utilities::detail

#endif // _WIN32
