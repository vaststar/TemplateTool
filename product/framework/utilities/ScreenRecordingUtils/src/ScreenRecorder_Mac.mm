#include "ScreenRecorder_Mac.h"

#ifdef __APPLE__

#include "LoggerDefine.h"

#include <cerrno>
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
// Recording — start / stop / pause / resume
// ============================================================================

ScreenRecorder_Mac::ScreenRecorder_Mac() = default;

ScreenRecorder_Mac::~ScreenRecorder_Mac()
{
    if (m_active.load())
    {
        stop();
    }
}

bool ScreenRecorder_Mac::isActive() const
{
    return m_active.load();
}

std::string ScreenRecorder_Mac::outputPath() const
{
    return m_outputPath;
}

bool ScreenRecorder_Mac::start(const RecordingConfig& config)
{
    if (m_active.load())
    {
        SRU_LOG_ERROR("start: already recording");
        return false;
    }

    m_outputPath = config.outputPath;

    if (config.ffmpegPath.empty() || config.outputPath.empty())
    {
        SRU_LOG_ERROR("start: ffmpegPath or outputPath is empty");
        return false;
    }

    // Pre-check screen recording permission to fail fast with a clear error
    // instead of launching an FFmpeg process that silently captures a black screen.
    if (!hasScreenRecordingPermission())
    {
        SRU_LOG_ERROR("start: screen recording permission not granted. "
                      "Go to System Settings > Privacy & Security > Screen Recording to enable.");
        return false;
    }

    // Create a pipe for FFmpeg's stdin so we can send 'q' to stop it
    int stdinPipe[2]; // [0]=read, [1]=write
    if (pipe(stdinPipe) < 0)
    {
        SRU_LOG_ERROR("start: pipe() failed: " << strerror(errno));
        return false;
    }
    setCloseOnExec(stdinPipe[0]);
    setCloseOnExec(stdinPipe[1]);

    // Prevent SIGPIPE when writing to this fd after FFmpeg exits
    fcntl(stdinPipe[1], F_SETNOSIGPIPE, 1);

    // Build FFmpeg command-line arguments
    // macOS screen capture: -f avfoundation -i "Capture screen <N>:<audio_device>"
    // We use the screen device name instead of a raw numeric index because
    // avfoundation lists cameras before screens (e.g. index 0 = FaceTime camera,
    // index 1 = Capture screen 0). Using the name avoids mis-targeting the camera.

    // Determine audio portion of the input spec
    // For MicAndSystem mode, we need two separate avfoundation inputs
    std::string audioInputName = "none";
    bool needSecondInput = false;
    std::string secondAudioInputName;

    switch (config.audioMode)
    {
    case AudioCaptureMode::None:
        audioInputName = "none";
        break;
    case AudioCaptureMode::Microphone:
        audioInputName = config.micDevice.empty() ? "default" : config.micDevice;
        break;
    case AudioCaptureMode::SystemAudio:
        audioInputName = config.systemAudioDevice.empty() ? "none" : config.systemAudioDevice;
        break;
    case AudioCaptureMode::MicAndSystem:
        // Primary input captures screen + microphone
        audioInputName = config.micDevice.empty() ? "default" : config.micDevice;
        // Second input captures system audio (requires a loopback device like BlackHole)
        if (!config.systemAudioDevice.empty())
        {
            needSecondInput = true;
            secondAudioInputName = config.systemAudioDevice;
        }
        break;
    }

    std::string inputSpec = "Capture screen " + std::to_string(config.displayIndex) + ":" + audioInputName;

    std::vector<std::string> args;
    args.push_back(config.ffmpegPath);
    args.push_back("-y");                          // overwrite output

    // Increase thread queue to prevent "Thread message queue blocking" drops
    args.push_back("-thread_queue_size");
    args.push_back("512");
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
    args.push_back("-probesize");
    args.push_back("32");
    args.push_back("-i");
    args.push_back(inputSpec);

    // Second avfoundation input for system audio (MicAndSystem mode)
    if (needSecondInput)
    {
        args.push_back("-thread_queue_size");
        args.push_back("512");
        args.push_back("-f");
        args.push_back("avfoundation");
        args.push_back("-i");
        args.push_back("none:" + secondAudioInputName);
    }

    // Do not synthesize CFR by duplicating frames from unstable input timestamps.
    args.push_back("-vsync");
    args.push_back("0");

    // Region crop filter and audio mixing filter
    std::string videoFilter;
    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        int w = alignToEven(config.regionW);
        int h = alignToEven(config.regionH);
        videoFilter = "crop=" + std::to_string(w) + ":"
                    + std::to_string(h) + ":"
                    + std::to_string(config.regionX) + ":"
                    + std::to_string(config.regionY);
    }

    if (needSecondInput)
    {
        // Use filter_complex to resample both audio streams to 48kHz stereo
        // before mixing — prevents stuttering from sample rate mismatch.
        std::string filterComplex;
        if (!videoFilter.empty())
        {
            filterComplex = "[0:v]" + videoFilter + "[vout];"
                "[0:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a0];"
                "[1:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a1];"
                "[a0][a1]amix=inputs=2:duration=first:normalize=0[aout]";
            args.push_back("-filter_complex");
            args.push_back(filterComplex);
            args.push_back("-map");
            args.push_back("[vout]");
            args.push_back("-map");
            args.push_back("[aout]");
        }
        else
        {
            filterComplex =
                "[0:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a0];"
                "[1:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a1];"
                "[a0][a1]amix=inputs=2:duration=first:normalize=0[aout]";
            args.push_back("-filter_complex");
            args.push_back(filterComplex);
            args.push_back("-map");
            args.push_back("0:v");
            args.push_back("-map");
            args.push_back("[aout]");
        }
    }
    else if (!videoFilter.empty())
    {
        args.push_back("-vf");
        args.push_back(videoFilter);
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
        args.push_back("-tune");
        args.push_back("zerolatency");
        args.push_back("-pix_fmt");
        args.push_back("yuv420p");

        // Improve mp4/mov compatibility with QuickTime players.
        if (config.videoFormat == "mp4" || config.videoFormat == "mov")
        {
            args.push_back("-video_track_timescale");
            args.push_back("600");
        }
    }

    // Audio codec settings (only when audio is enabled)
    if (config.audioMode != AudioCaptureMode::None)
    {
        // Ensure consistent output sample rate
        args.push_back("-ar");
        args.push_back("48000");
        args.push_back("-ac");
        args.push_back("2");

        args.push_back("-c:a");
        if (config.videoFormat == "webm")
        {
            args.push_back("libopus");
            args.push_back("-b:a");
            args.push_back("128k");
        }
        else
        {
            args.push_back("aac");
            args.push_back("-b:a");
            args.push_back("128k");
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
        SRU_LOG_ERROR("start: posix_spawn failed: " << strerror(spawnResult));
        close(stdinPipe[1]);
        return false;
    }

    m_pid = pid;
    m_stdinFd = stdinPipe[1];
    m_active.store(true);

    SRU_LOG_INFO("start: started ffmpeg pid=" << pid << " output=" << config.outputPath);
    return true;
}

RecordingResult ScreenRecorder_Mac::stop()
{
    RecordingResult result;
    result.outputPath = m_outputPath;

    if (!m_active.load())
    {
        result.errorMessage = "Not recording";
        return result;
    }

    // Send 'q' to FFmpeg's stdin to gracefully stop.
    if (m_stdinFd >= 0)
    {
        const char quitCmd[] = "q\n";
        ssize_t written = write(m_stdinFd, quitCmd, sizeof(quitCmd) - 1);
        if (written < 0 && errno != EPIPE)
        {
            SRU_LOG_WARN("stop: write to stdin pipe failed: " << strerror(errno));
        }

        close(m_stdinFd);
        m_stdinFd = -1;
    }

    // Wait for process to exit (up to 10 seconds, then escalate)
    int status = 0;
    std::string killError;
    waitForChildWithTimeout(static_cast<pid_t>(m_pid), status, 10000, killError);

    if (!killError.empty())
    {
        result.errorMessage = killError;
    }

    m_pid = -1;
    m_active.store(false);

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
        SRU_LOG_ERROR("stop: " << result.errorMessage);
    }

    return result;
}

bool ScreenRecorder_Mac::pause()
{
    if (!m_active.load())
    {
        return false;
    }
    return kill(static_cast<pid_t>(m_pid), SIGSTOP) == 0;
}

bool ScreenRecorder_Mac::resume()
{
    if (!m_active.load())
    {
        return false;
    }
    return kill(static_cast<pid_t>(m_pid), SIGCONT) == 0;
}

} // namespace ucf::utilities::screenrecording

#endif // __APPLE__
