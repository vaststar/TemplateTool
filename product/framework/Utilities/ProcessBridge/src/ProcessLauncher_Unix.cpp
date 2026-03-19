#if !defined(_WIN32)

#include "ProcessLauncher.h"

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <sstream>
#include <thread>

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ucf::utilities::detail {

ProcessLauncher::ProcessHandle ProcessLauncher::launch(
    const std::string& executable,
    const std::vector<std::string>& args,
    const std::string& workingDir)
{
    ProcessHandle handle;

    // ── Create pipes ──
    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};

    if (pipe(stdoutPipe) != 0)
    {
        handle.errorMessage = std::string("Failed to create stdout pipe: ") + strerror(errno);
        return handle;
    }
    if (pipe(stderrPipe) != 0)
    {
        handle.errorMessage = std::string("Failed to create stderr pipe: ") + strerror(errno);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        return handle;
    }

    // ── Build argv ──
    std::vector<const char*> argv;
    argv.push_back(executable.c_str());
    for (const auto& a : args)
    {
        argv.push_back(a.c_str());
    }
    argv.push_back(nullptr);

    // ── Fork ──
    pid_t pid = fork();
    if (pid < 0)
    {
        handle.errorMessage = std::string("fork failed: ") + strerror(errno);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[0]);
        close(stderrPipe[1]);
        return handle;
    }

    if (pid == 0)
    {
        // ── Child process ──
        // Create a new process group so signals sent to the child
        // do not propagate to the parent (mirrors CREATE_NEW_PROCESS_GROUP on Windows).
        setpgid(0, 0);

        close(stdoutPipe[0]);
        dup2(stdoutPipe[1], STDOUT_FILENO);
        close(stdoutPipe[1]);

        close(stderrPipe[0]);
        dup2(stderrPipe[1], STDERR_FILENO);
        close(stderrPipe[1]);

        if (!workingDir.empty())
        {
            if (chdir(workingDir.c_str()) != 0)
            {
                _exit(127);
            }
        }

        execvp(executable.c_str(), const_cast<char* const*>(argv.data()));
        // If execvp returns, it failed
        _exit(127);
    }

    // ── Parent process ──
    // Close write ends
    close(stdoutPipe[1]);
    close(stderrPipe[1]);

    // Set read ends to non-blocking
    int flags = fcntl(stdoutPipe[0], F_GETFL, 0);
    fcntl(stdoutPipe[0], F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(stderrPipe[0], F_GETFL, 0);
    fcntl(stderrPipe[0], F_SETFL, flags | O_NONBLOCK);

    handle.valid = true;
    handle.pid = static_cast<int64_t>(pid);
    handle.childPid = pid;
    handle.stdoutFd = stdoutPipe[0];
    handle.stderrFd = stderrPipe[0];
    return handle;
}

bool ProcessLauncher::terminate(const ProcessHandle& handle)
{
    if (!handle.valid || handle.childPid <= 0)
    {
        return false;
    }
    return ::kill(handle.childPid, SIGTERM) == 0;
}

bool ProcessLauncher::kill(const ProcessHandle& handle)
{
    if (!handle.valid || handle.childPid <= 0)
    {
        return false;
    }
    return ::kill(handle.childPid, SIGKILL) == 0;
}

bool ProcessLauncher::isAlive(ProcessHandle& handle)
{
    if (!handle.valid || handle.childPid <= 0)
    {
        return false;
    }
    if (handle.exited)
    {
        return false;
    }
    int status = 0;
    pid_t result = waitpid(handle.childPid, &status, WNOHANG);
    if (result > 0)
    {
        // Child has exited — cache the exit code so waitForExit can retrieve it.
        handle.exited = true;
        if (WIFEXITED(status))
        {
            handle.cachedExitCode = WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status))
        {
            handle.cachedExitCode = 128 + WTERMSIG(status);
        }
        return false;
    }
    return result == 0;
}

int ProcessLauncher::waitForExit(ProcessHandle& handle, int timeoutMs)
{
    if (!handle.valid || handle.childPid <= 0)
    {
        return -1;
    }
    // If isAlive() already reaped the child, return the cached exit code.
    if (handle.exited)
    {
        return handle.cachedExitCode;
    }

    if (timeoutMs < 0)
    {
        // Infinite wait
        int status = 0;
        waitpid(handle.childPid, &status, 0);
        handle.exited = true;
        if (WIFEXITED(status))
        {
            return handle.cachedExitCode = WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status))
        {
            return handle.cachedExitCode = 128 + WTERMSIG(status);
        }
        return handle.cachedExitCode = -1;
    }

    // Poll-based timeout wait using steady_clock for accurate timing
    auto deadline = std::chrono::steady_clock::now()
                  + std::chrono::milliseconds(timeoutMs);
    constexpr auto pollInterval = std::chrono::milliseconds(50);

    while (std::chrono::steady_clock::now() < deadline)
    {
        int status = 0;
        pid_t result = waitpid(handle.childPid, &status, WNOHANG);
        if (result > 0)
        {
            handle.exited = true;
            if (WIFEXITED(status))
            {
                return handle.cachedExitCode = WEXITSTATUS(status);
            }
            if (WIFSIGNALED(status))
            {
                return handle.cachedExitCode = 128 + WTERMSIG(status);
            }
            return handle.cachedExitCode = -1;
        }

        std::this_thread::sleep_for(pollInterval);
    }
    return -1;  // timeout
}

static std::string readFd(int fd)
{
    if (fd < 0)
    {
        return {};
    }

    char buf[4096];
    std::string result;
    while (true)
    {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0)
        {
            result.append(buf, static_cast<size_t>(n));
        }
        else
        {
            break;  // EAGAIN (no data) or EOF or error
        }
    }
    return result;
}

std::string ProcessLauncher::readStdout(ProcessHandle& handle)
{
    return readFd(handle.stdoutFd);
}

std::string ProcessLauncher::readStderr(ProcessHandle& handle)
{
    return readFd(handle.stderrFd);
}

void ProcessLauncher::closeHandles(ProcessHandle& handle)
{
    if (handle.stdoutFd >= 0)
    {
        close(handle.stdoutFd);
        handle.stdoutFd = -1;
    }
    if (handle.stderrFd >= 0)
    {
        close(handle.stderrFd);
        handle.stderrFd = -1;
    }
    handle.valid = false;
}

} // namespace ucf::utilities::detail

#endif // !_WIN32
