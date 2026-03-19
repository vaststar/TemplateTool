#ifdef _WIN32

#include "ProcessLauncher.h"

#include <sstream>

namespace ucf::utilities::detail {

/// Escape a single argument for Windows command line (MSVC CRT parsing rules).
static std::string escapeArgument(const std::string& arg)
{
    if (!arg.empty() && arg.find_first_of(" \t\"\\" ) == std::string::npos)
    {
        return arg;
    }

    std::string result = "\"";
    for (auto it = arg.begin(); ; ++it)
    {
        int numBackslashes = 0;
        while (it != arg.end() && *it == '\\')
        {
            ++it;
            ++numBackslashes;
        }

        if (it == arg.end())
        {
            // Trailing backslashes must be doubled (preceding closing quote)
            result.append(static_cast<size_t>(numBackslashes * 2), '\\');
            break;
        }
        else if (*it == '"')
        {
            // Backslashes before " are doubled, plus one escape backslash
            result.append(static_cast<size_t>(numBackslashes * 2 + 1), '\\');
            result.push_back('"');
        }
        else
        {
            result.append(static_cast<size_t>(numBackslashes), '\\');
            result.push_back(*it);
        }
    }
    result.push_back('"');
    return result;
}

ProcessLauncher::ProcessHandle ProcessLauncher::launch(
    const std::string& executable,
    const std::vector<std::string>& args,
    const std::string& workingDir)
{
    ProcessHandle handle;

    // ── Build command line ──
    std::ostringstream cmdLine;
    cmdLine << escapeArgument(executable);
    for (const auto& arg : args)
    {
        cmdLine << " " << escapeArgument(arg);
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

    if (!CreatePipe(&hStdoutReadTmp, &hStdoutWrite, &sa, 0))
    {
        handle.errorMessage = "Failed to create stdout pipe";
        return handle;
    }
    // Ensure the read end is not inherited
    SetHandleInformation(hStdoutReadTmp, HANDLE_FLAG_INHERIT, 0);

    if (!CreatePipe(&hStderrReadTmp, &hStderrWrite, &sa, 0))
    {
        handle.errorMessage = "Failed to create stderr pipe";
        CloseHandle(hStdoutReadTmp);
        CloseHandle(hStdoutWrite);
        return handle;
    }
    SetHandleInformation(hStderrReadTmp, HANDLE_FLAG_INHERIT, 0);

    // ── Launch process ──
    STARTUPINFOW si{};
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = hStdoutWrite;
    si.hStdError  = hStderrWrite;

    PROCESS_INFORMATION pi{};
    DWORD creationFlags = CREATE_NO_WINDOW | CREATE_NEW_PROCESS_GROUP;

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
    CloseHandle(hStdoutWrite);
    CloseHandle(hStderrWrite);

    if (!ok)
    {
        DWORD err = GetLastError();
        handle.errorMessage = "CreateProcess failed, error code: " + std::to_string(err);
        CloseHandle(hStdoutReadTmp);
        CloseHandle(hStderrReadTmp);
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
    // Try graceful shutdown via console control event (works for console apps).
    // Falls back to TerminateProcess if the signal cannot be delivered.
    if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, static_cast<DWORD>(handle.pid)))
    {
        return true;
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

bool ProcessLauncher::isAlive(ProcessHandle& handle)
{
    if (!handle.valid || !handle.hProcess)
    {
        return false;
    }
    // Use WaitForSingleObject with 0 timeout instead of GetExitCodeProcess
    // to avoid the STILL_ACTIVE (259) exit code false-positive.
    return WaitForSingleObject(handle.hProcess, 0) == WAIT_TIMEOUT;
}

int ProcessLauncher::waitForExit(ProcessHandle& handle, int timeoutMs)
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

std::string ProcessLauncher::readStdout(ProcessHandle& handle)
{
    return readPipe(handle.hStdoutRead);
}

std::string ProcessLauncher::readStderr(ProcessHandle& handle)
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
