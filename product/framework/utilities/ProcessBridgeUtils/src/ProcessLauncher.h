#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ucf::utilities::detail {

/// Low-level cross-platform child process launcher.
/// This is an internal helper — not part of the public API.
class ProcessLauncher
{
public:
    /// Handle to a launched process and its I/O pipes.
    struct ProcessHandle
    {
        bool valid = false;
        int64_t pid = 0;
        std::string errorMessage;

#ifdef _WIN32
        HANDLE hProcess = nullptr;
        HANDLE hThread  = nullptr;
        HANDLE hStdoutRead = nullptr;
        HANDLE hStderrRead = nullptr;
        HANDLE hStdinWrite = nullptr;  ///< Write end of stdin pipe (parent keeps this)
#else
        int childPid = -1;
        int stdoutFd = -1;
        int stderrFd = -1;
        int stdinFd  = -1;        ///< Write end of stdin pipe (parent keeps this)
        int cachedExitCode = -1;  ///< exit code set once waitpid reaps the child
        bool exited = false;     ///< true once waitpid has reaped the child
#endif
    };

    /// Launch a child process.
    /// @param executable    Path to the executable
    /// @param args          Command-line arguments
    /// @param workingDir    Working directory (empty = inherit)
    /// @param pipeStdin     If true, create a stdin pipe so the parent can write to the child's stdin
    /// @param environment   Optional environment variables for the child process.
    ///                      If empty, the child inherits the parent's environment.
    /// @return Handle with process info; check handle.valid
    static ProcessHandle launch(
        const std::string& executable,
        const std::vector<std::string>& args,
        const std::string& workingDir,
        bool pipeStdin = false,
        const std::vector<std::pair<std::string, std::string>>& environment = {});

    /// Send terminate signal (SIGTERM / TerminateProcess).
    static bool terminate(const ProcessHandle& handle);

    /// Force kill (SIGKILL / TerminateProcess with exit code 1).
    static bool kill(const ProcessHandle& handle);

    /// Check if the process is still alive.
    static bool isAlive(ProcessHandle& handle);

    /// Wait for the process to exit.
    /// @param timeoutMs  Maximum wait time (-1 = infinite)
    /// @return exit code, or -1 if timeout
    static int waitForExit(ProcessHandle& handle, int timeoutMs);

    /// Read available data from stdout pipe (non-blocking).
    /// @return data read, empty if nothing available
    static std::string readStdout(ProcessHandle& handle);

    /// Read available data from stderr pipe (non-blocking).
    /// @return data read, empty if nothing available
    static std::string readStderr(ProcessHandle& handle);

    /// Write data to the child's stdin pipe.
    /// @return true if all bytes were written successfully
    static bool writeStdin(ProcessHandle& handle, const std::string& data);

    /// Close the stdin pipe (child sees EOF on its stdin).
    static void closeStdin(ProcessHandle& handle);

    /// Clean up all handles/file descriptors.
    static void closeHandles(ProcessHandle& handle);
};

} // namespace ucf::utilities::detail
