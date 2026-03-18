#if !defined(_WIN32)

#include "ProcessLauncher.h"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <sstream>

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ucf::utilities::detail {

ProcessLauncher::ProcessHandle ProcessLauncher::launch(
    const std::string& executable,
    const std::vector<std::string>& args,
    const std::string& workingDir,
    bool captureStdout,
    bool captureStderr)
{
    ProcessHandle handle;

    // ── Create pipes ──
    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};

    if (captureStdout && pipe(stdoutPipe) != 0)
    {
        handle.errorMessage = std::string("Failed to create stdout pipe: ") + strerror(errno);
        return handle;
    }
    if (captureStderr && pipe(stderrPipe) != 0)
    {
        handle.errorMessage = std::string("Failed to create stderr pipe: ") + strerror(errno);
        if (captureStdout)
        {
            close(stdoutPipe[0]);
            close(stdoutPipe[1]);
        }
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
        if (captureStdout)
        {
            close(stdoutPipe[0]);
            close(stdoutPipe[1]);
        }
        if (captureStderr)
        {
            close(stderrPipe[0]);
            close(stderrPipe[1]);
        }
        return handle;
    }

    if (pid == 0)
    {
        // ── Child process ──
        if (captureStdout)
        {
            close(stdoutPipe[0]);
            dup2(stdoutPipe[1], STDOUT_FILENO);
            close(stdoutPipe[1]);
        }
        if (captureStderr)
        {
            close(stderrPipe[0]);
            dup2(stderrPipe[1], STDERR_FILENO);
            close(stderrPipe[1]);
        }

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
    if (captureStdout)
    {
        close(stdoutPipe[1]);
    }
    if (captureStderr)
    {
        close(stderrPipe[1]);
    }

    // Set read ends to non-blocking
    if (captureStdout)
    {
        int flags = fcntl(stdoutPipe[0], F_GETFL, 0);
        fcntl(stdoutPipe[0], F_SETFL, flags | O_NONBLOCK);
    }
    if (captureStderr)
    {
        int flags = fcntl(stderrPipe[0], F_GETFL, 0);
        fcntl(stderrPipe[0], F_SETFL, flags | O_NONBLOCK);
    }

    handle.valid = true;
    handle.pid = static_cast<int64_t>(pid);
    handle.childPid = pid;
    handle.stdoutFd = captureStdout ? stdoutPipe[0] : -1;
    handle.stderrFd = captureStderr ? stderrPipe[0] : -1;
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

bool ProcessLauncher::isAlive(const ProcessHandle& handle)
{
    if (!handle.valid || handle.childPid <= 0)
    {
        return false;
    }
    int status = 0;
    pid_t result = waitpid(handle.childPid, &status, WNOHANG);
    return result == 0;
}

int ProcessLauncher::waitForExit(const ProcessHandle& handle, int timeoutMs)
{
    if (!handle.valid || handle.childPid <= 0)
    {
        return -1;
    }

    if (timeoutMs < 0)
    {
        // Infinite wait
        int status = 0;
        waitpid(handle.childPid, &status, 0);
        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        return -1;
    }

    // Poll-based timeout wait
    int elapsed = 0;
    constexpr int pollIntervalMs = 50;
    while (elapsed < timeoutMs)
    {
        int status = 0;
        pid_t result = waitpid(handle.childPid, &status, WNOHANG);
        if (result > 0)
        {
            if (WIFEXITED(status))
            {
                return WEXITSTATUS(status);
            }
            if (WIFSIGNALED(status))
            {
                return 128 + WTERMSIG(status);
            }
            return -1;
        }

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = pollIntervalMs * 1000000L;
        nanosleep(&ts, nullptr);
        elapsed += pollIntervalMs;
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

std::string ProcessLauncher::readStdout(const ProcessHandle& handle)
{
    return readFd(handle.stdoutFd);
}

std::string ProcessLauncher::readStderr(const ProcessHandle& handle)
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
