#include "ScreenRecordingUtils_Mac.h"

#ifdef __APPLE__

#include "LoggerDefine.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <CoreGraphics/CGDirectDisplay.h>

extern char** environ;

namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers
// ============================================================================

/// Round a dimension down to the nearest even number (minimum 2).
/// libx264 + yuv420p requires even width and height.
static inline int alignToEven(int v)
{
    return std::max(2, v & ~1);
}

/// Set close-on-exec flag on a file descriptor.
static inline void setCloseOnExec(int fd)
{
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
}

/// Search the PATH environment variable for an executable by name.
static std::string findInPath(const std::string& name)
{
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv)
    {
        return {};
    }

    std::string pathStr(pathEnv);
    std::string::size_type start = 0;

    while (start < pathStr.size())
    {
        auto end = pathStr.find(':', start);
        if (end == std::string::npos)
        {
            end = pathStr.size();
        }

        std::string dir = pathStr.substr(start, end - start);
        if (!dir.empty())
        {
            std::filesystem::path candidate = std::filesystem::path(dir) / name;
            std::error_code ec;
            if (std::filesystem::is_regular_file(candidate, ec) && access(candidate.c_str(), X_OK) == 0)
            {
                auto canonical = std::filesystem::canonical(candidate, ec);
                if (!ec)
                {
                    return canonical.string();
                }
            }
        }
        start = end + 1;
    }
    return {};
}

/// Wait for a child process with timeout, escalating from SIGINT to SIGKILL.
/// @param pid        Child PID
/// @param status     Output: exit status
/// @param timeoutMs  Total timeout in milliseconds before first escalation
/// @param errorMsg   Output: set if force-killed
/// @return true if process exited normally, false if force-killed
static bool waitForChildWithTimeout(pid_t pid, int& status, int timeoutMs, std::string& errorMsg)
{
    constexpr int pollIntervalMs = 100;

    // Phase 1: wait for voluntary exit
    int elapsed = 0;
    while (elapsed < timeoutMs)
    {
        pid_t wpid = waitpid(pid, &status, WNOHANG);
        if (wpid != 0)
        {
            return true;
        }
        usleep(pollIntervalMs * 1000);
        elapsed += pollIntervalMs;
    }

    // Phase 2: SIGINT for graceful muxer finalization (5 seconds)
    SRU_LOG_WARN("Child process " << pid << " did not exit in " << timeoutMs << "ms, sending SIGINT");
    kill(pid, SIGINT);

    constexpr int intTimeoutMs = 5000;
    elapsed = 0;
    while (elapsed < intTimeoutMs)
    {
        pid_t wpid = waitpid(pid, &status, WNOHANG);
        if (wpid != 0)
        {
            return true;
        }
        usleep(pollIntervalMs * 1000);
        elapsed += pollIntervalMs;
    }

    // Phase 3: SIGKILL as last resort
    SRU_LOG_ERROR("Child process " << pid << " did not respond to SIGINT, sending SIGKILL");
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    errorMsg = "FFmpeg did not exit in time, force killed";
    return false;
}

// ============================================================================
// FFmpeg Discovery
// ============================================================================

std::string ScreenRecordingUtils_Mac::findFFmpegPath(const std::string& appDir)
{
    // On macOS, appDir = .../mainEntry.app/Contents/MacOS
    // ffmpeg sits in .../bin/ffmpeg (next to the .app bundle), so go up 3 levels.
    std::vector<std::string> candidates = {
        appDir + "/../../../ffmpeg",          // build output: bin/ffmpeg (next to .app)
        appDir + "/ffmpeg",                   // flat layout
        appDir + "/../Resources/ffmpeg",      // bundled inside .app
        "/opt/homebrew/bin/ffmpeg",
        "/usr/local/bin/ffmpeg"
    };

    for (const auto& candidate : candidates)
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec))
        {
            return canonical.string();
        }
    }

    // Fallback: search PATH manually (no shell invocation)
    std::string pathResult = findInPath("ffmpeg");
    if (!pathResult.empty())
    {
        return pathResult;
    }

    SRU_LOG_WARN("FFmpeg not found in candidates or PATH");
    return {};
}

// ============================================================================
// Permission Check
// ============================================================================

bool ScreenRecordingUtils_Mac::hasScreenRecordingPermission()
{
    // CGPreflightScreenCaptureAccess() is available on macOS 10.15+.
    // Returns true if the app already has screen recording permission.
    // Does NOT trigger the system permission dialog.
    if (__builtin_available(macOS 10.15, *))
    {
        return CGPreflightScreenCaptureAccess();
    }
    // Before macOS 10.15, no permission was required.
    return true;
}

// ============================================================================
// Recording — start / stop / pause / resume
// ============================================================================

RecordingSession ScreenRecordingUtils_Mac::startRecording(const RecordingConfig& config)
{
    RecordingSession session;
    session.outputPath = config.outputPath;

    if (config.ffmpegPath.empty() || config.outputPath.empty())
    {
        SRU_LOG_ERROR("startRecording: ffmpegPath or outputPath is empty");
        return session; // invalid
    }

    // Pre-check screen recording permission to fail fast with a clear error
    // instead of launching an FFmpeg process that silently captures a black screen.
    if (!hasScreenRecordingPermission())
    {
        SRU_LOG_ERROR("startRecording: screen recording permission not granted. "
                      "Go to System Settings > Privacy & Security > Screen Recording to enable.");
        return session;
    }

    // Create a pipe for FFmpeg's stdin so we can send 'q' to stop it
    int stdinPipe[2]; // [0]=read, [1]=write
    if (pipe(stdinPipe) < 0)
    {
        SRU_LOG_ERROR("startRecording: pipe() failed: " << strerror(errno));
        return session;
    }
    setCloseOnExec(stdinPipe[0]);
    setCloseOnExec(stdinPipe[1]);

    // Prevent SIGPIPE when writing to this fd after FFmpeg exits
    fcntl(stdinPipe[1], F_SETNOSIGPIPE, 1);

    // Build FFmpeg command-line arguments
    // macOS screen capture: -f avfoundation -i "Capture screen <N>:none"
    // We use the screen device name instead of a raw numeric index because
    // avfoundation lists cameras before screens (e.g. index 0 = FaceTime camera,
    // index 1 = Capture screen 0). Using the name avoids mis-targeting the camera.
    std::string inputSpec = "Capture screen " + std::to_string(config.displayIndex) + ":none";

    std::vector<std::string> args;
    args.push_back(config.ffmpegPath);
    args.push_back("-y");                          // overwrite output
    args.push_back("-f");
    args.push_back("avfoundation");
    args.push_back("-framerate");
    args.push_back(std::to_string(config.fps));
    args.push_back("-capture_cursor");
    args.push_back("1");
    // avfoundation may expose unstable stream timestamps for screen capture.
    // Use wallclock timestamps to avoid huge duplicate-frame bursts.
    args.push_back("-use_wallclock_as_timestamps");
    args.push_back("1");
    args.push_back("-i");
    args.push_back(inputSpec);

    // Do not synthesize CFR by duplicating frames from unstable input timestamps.
    args.push_back("-vsync");
    args.push_back("0");

    // Region crop filter
    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        int w = alignToEven(config.regionW);
        int h = alignToEven(config.regionH);
        std::string cropFilter = "crop=" + std::to_string(w) + ":"
                                + std::to_string(h) + ":"
                                + std::to_string(config.regionX) + ":"
                                + std::to_string(config.regionY);
        args.push_back("-vf");
        args.push_back(cropFilter);
    }

    // Output codec settings
    args.push_back("-c:v");
    if (config.videoFormat == "webm")
    {
        args.push_back("libvpx-vp9");
        args.push_back("-b:v");
        args.push_back("2M");
    }
    else
    {
        args.push_back("libx264");
        args.push_back("-preset");
        args.push_back("ultrafast");
        args.push_back("-pix_fmt");
        args.push_back("yuv420p");

        // Improve mp4/mov compatibility with QuickTime players.
        if (config.videoFormat == "mp4" || config.videoFormat == "mov")
        {
            args.push_back("-video_track_timescale");
            args.push_back("600");
        }
    }

    args.push_back(config.outputPath);

    // Convert to C-style argv for posix_spawn
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (auto& a : args)
    {
        argv.push_back(a.data());
    }
    argv.push_back(nullptr);

    // Set up file actions for the child process
    posix_spawn_file_actions_t fileActions;
    posix_spawn_file_actions_init(&fileActions);

    // Redirect stdin from the pipe read end
    posix_spawn_file_actions_adddup2(&fileActions, stdinPipe[0], STDIN_FILENO);
    posix_spawn_file_actions_addclose(&fileActions, stdinPipe[0]);
    posix_spawn_file_actions_addclose(&fileActions, stdinPipe[1]);

    // Redirect stdout to /dev/null
    posix_spawn_file_actions_addopen(&fileActions, STDOUT_FILENO,
                                     "/dev/null", O_WRONLY, 0);

    // Redirect stderr to a log file for troubleshooting
    std::string logPath = config.outputPath + ".ffmpeg.log";
    posix_spawn_file_actions_addopen(&fileActions, STDERR_FILENO,
                                     logPath.c_str(),
                                     O_WRONLY | O_CREAT | O_TRUNC, 0644);

    pid_t pid = 0;
    int spawnResult = posix_spawn(&pid, config.ffmpegPath.c_str(),
                                  &fileActions, nullptr,
                                  argv.data(), environ);
    posix_spawn_file_actions_destroy(&fileActions);

    // Close read end in parent (write end kept for sending 'q')
    close(stdinPipe[0]);

    if (spawnResult != 0)
    {
        SRU_LOG_ERROR("startRecording: posix_spawn failed: " << strerror(spawnResult));
        close(stdinPipe[1]);
        return session;
    }

    session.pid = pid;
    session.stdinFd = stdinPipe[1];

    SRU_LOG_INFO("startRecording: started ffmpeg pid=" << pid << " output=" << config.outputPath);
    return session;
}

RecordingResult ScreenRecordingUtils_Mac::stopRecording(RecordingSession& session)
{
    RecordingResult result;
    result.outputPath = session.outputPath;

    if (!session.isValid())
    {
        result.errorMessage = "Invalid session";
        SRU_LOG_ERROR("stopRecording: " << result.errorMessage);
        return result;
    }

    // Send 'q' to FFmpeg's stdin to gracefully stop.
    // F_SETNOSIGPIPE was set on this fd in startRecording, so write() returns
    // EPIPE instead of raising SIGPIPE if FFmpeg has already exited.
    if (session.stdinFd >= 0)
    {
        const char quitCmd[] = "q\n";
        ssize_t written = write(session.stdinFd, quitCmd, sizeof(quitCmd) - 1);
        if (written < 0 && errno != EPIPE)
        {
            SRU_LOG_WARN("stopRecording: write to stdin pipe failed: " << strerror(errno));
        }

        close(session.stdinFd);
        session.stdinFd = -1;
    }

    // Wait for process to exit (up to 10 seconds, then escalate)
    int status = 0;
    std::string killError;
    waitForChildWithTimeout(static_cast<pid_t>(session.pid), status, 10000, killError);

    if (!killError.empty())
    {
        result.errorMessage = killError;
    }

    session.pid = -1;

    // Check if output file exists
    std::error_code ec;
    if (std::filesystem::is_regular_file(result.outputPath, ec))
    {
        result.success = true;
    }
    else
    {
        if (result.errorMessage.empty())
        {
            result.errorMessage = "Output file not found: " + result.outputPath;
        }
        SRU_LOG_ERROR("stopRecording: " << result.errorMessage);
    }

    return result;
}

bool ScreenRecordingUtils_Mac::pauseRecording(const RecordingSession& session)
{
    if (!session.isValid())
    {
        return false;
    }
    return kill(static_cast<pid_t>(session.pid), SIGSTOP) == 0;
}

bool ScreenRecordingUtils_Mac::resumeRecording(const RecordingSession& session)
{
    if (!session.isValid())
    {
        return false;
    }
    return kill(static_cast<pid_t>(session.pid), SIGCONT) == 0;
}

// ============================================================================
// GIF Conversion
// ============================================================================

bool ScreenRecordingUtils_Mac::convertToGif(const std::string& ffmpegPath,
                                            const std::string& inputPath,
                                            const std::string& outputPath,
                                            int fps)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
    {
        SRU_LOG_ERROR("convertToGif: empty path argument");
        return false;
    }

    // Build palette + GIF two-pass command as a single filter_complex
    std::string filterComplex = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    // Build argv
    std::vector<std::string> args = {
        ffmpegPath, "-y", "-i", inputPath,
        "-filter_complex", filterComplex,
        outputPath
    };
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (auto& a : args)
    {
        argv.push_back(a.data());
    }
    argv.push_back(nullptr);

    // Set up file actions: redirect stdout and stderr to /dev/null
    posix_spawn_file_actions_t fileActions;
    posix_spawn_file_actions_init(&fileActions);
    posix_spawn_file_actions_addopen(&fileActions, STDOUT_FILENO,
                                     "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&fileActions, STDERR_FILENO,
                                     "/dev/null", O_WRONLY, 0);

    pid_t pid = 0;
    int spawnResult = posix_spawn(&pid, ffmpegPath.c_str(),
                                  &fileActions, nullptr,
                                  argv.data(), environ);
    posix_spawn_file_actions_destroy(&fileActions);

    if (spawnResult != 0)
    {
        SRU_LOG_ERROR("convertToGif: posix_spawn failed: " << strerror(spawnResult));
        return false;
    }

    // Wait with timeout (120 seconds for potentially long GIF conversions)
    int status = 0;
    std::string killError;
    bool exited = waitForChildWithTimeout(pid, status, 120000, killError);

    if (!exited)
    {
        SRU_LOG_ERROR("convertToGif: " << killError);
        return false;
    }

    std::error_code ec;
    bool ok = WIFEXITED(status) && WEXITSTATUS(status) == 0
              && std::filesystem::is_regular_file(outputPath, ec);
    if (!ok)
    {
        SRU_LOG_ERROR("convertToGif: ffmpeg exited with status "
                      << (WIFEXITED(status) ? WEXITSTATUS(status) : -1));
    }
    return ok;
}

} // namespace ucf::utilities::screenrecording

#endif // __APPLE__
