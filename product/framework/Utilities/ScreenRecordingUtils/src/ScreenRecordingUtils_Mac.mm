#include "ScreenRecordingUtils_Mac.h"

#ifdef __APPLE__

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <array>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

namespace fs = std::filesystem;

namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers
// ============================================================================

static bool fileExists(const std::string& path)
{
    struct stat st{};
    return (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode));
}

/// Resolve a path that may contain ".." to its canonical form.
static std::string resolvePath(const std::string& path)
{
    char resolved[PATH_MAX];
    if (realpath(path.c_str(), resolved)) {
        return std::string(resolved);
    }
    return {};
}

/// Run a command synchronously and capture stdout (first line).
static std::string execCommand(const char* cmd)
{
    std::array<char, 256> buffer{};
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return {};
    if (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        result = buffer.data();
        // trim trailing whitespace
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) {
            result.pop_back();
        }
    }
    pclose(pipe);
    return result;
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

    for (const auto& candidate : candidates) {
        std::string resolved = resolvePath(candidate);
        if (!resolved.empty() && fileExists(resolved)) {
            return resolved;
        }
    }

    // Fallback: search PATH via 'which'
    std::string whichResult = execCommand("which ffmpeg 2>/dev/null");
    if (!whichResult.empty() && fileExists(whichResult)) {
        return whichResult;
    }

    return {};
}

// ============================================================================
// Recording — start / stop / pause / resume
// ============================================================================

RecordingSession ScreenRecordingUtils_Mac::startRecording(const RecordingConfig& config)
{
    RecordingSession session;
    session.outputPath = config.outputPath;

    if (config.ffmpegPath.empty() || config.outputPath.empty()) {
        return session; // invalid
    }

    // Create a pipe for FFmpeg's stdin so we can send 'q' to stop it
    int stdinPipe[2]; // [0]=read, [1]=write
    if (pipe(stdinPipe) < 0) {
        return session;
    }

    // Build FFmpeg command-line arguments
    // macOS screen capture: -f avfoundation -i "Capture screen <N>:none"
    // We use the screen device name instead of a raw numeric index because
    // avfoundation lists cameras before screens (e.g. index 0 = FaceTime camera,
    // index 1 = Capture screen 0). Using the name avoids mis-targeting the camera.
    std::string inputSpec;
    if (config.isRegion) {
        inputSpec = "Capture screen " + std::to_string(config.displayIndex) + ":none";
    } else {
        inputSpec = "Capture screen " + std::to_string(config.displayIndex) + ":none";
    }

    std::vector<std::string> args;
    args.push_back(config.ffmpegPath);
    args.push_back("-y");                          // overwrite output
    args.push_back("-f");
    args.push_back("avfoundation");
    args.push_back("-framerate");
    args.push_back(std::to_string(config.fps));
    args.push_back("-capture_cursor");
    args.push_back("1");
    args.push_back("-i");
    args.push_back(inputSpec);

    // Region crop filter
    if (config.isRegion && config.regionW > 0 && config.regionH > 0) {
        std::string cropFilter = "crop=" + std::to_string(config.regionW) + ":"
                                + std::to_string(config.regionH) + ":"
                                + std::to_string(config.regionX) + ":"
                                + std::to_string(config.regionY);
        args.push_back("-vf");
        args.push_back(cropFilter);
    }

    // Output codec settings
    args.push_back("-c:v");
    if (config.videoFormat == "webm") {
        args.push_back("libvpx-vp9");
        args.push_back("-b:v");
        args.push_back("2M");
    } else {
        args.push_back("libx264");
        args.push_back("-preset");
        args.push_back("ultrafast");
        args.push_back("-pix_fmt");
        args.push_back("yuv420p");
    }

    args.push_back(config.outputPath);

    // Convert to C-style argv
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (auto& a : args) {
        argv.push_back(a.data());
    }
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        return session;
    }

    if (pid == 0) {
        // Child process
        close(stdinPipe[1]); // close write end
        dup2(stdinPipe[0], STDIN_FILENO);
        close(stdinPipe[0]);

        // Redirect stdout to /dev/null, stderr to a log file for debugging
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            close(devnull);
        }

        // Write FFmpeg stderr to a log file for troubleshooting
        std::string logPath = config.outputPath + ".ffmpeg.log";
        int logFd = open(logPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (logFd >= 0) {
            dup2(logFd, STDERR_FILENO);
            close(logFd);
        }

        execv(config.ffmpegPath.c_str(), argv.data());
        _exit(127); // exec failed
    }

    // Parent process
    close(stdinPipe[0]); // close read end

    // Prevent SIGPIPE when writing to this fd after FFmpeg exits
#ifdef F_SETNOSIGPIPE
    fcntl(stdinPipe[1], F_SETNOSIGPIPE, 1);
#endif

    session.pid = pid;
    session.stdinFd = stdinPipe[1];

    return session;
}

RecordingResult ScreenRecordingUtils_Mac::stopRecording(RecordingSession& session)
{
    RecordingResult result;
    result.outputPath = session.outputPath;

    if (!session.isValid()) {
        result.errorMessage = "Invalid session";
        return result;
    }

    // Send 'q' to FFmpeg's stdin to gracefully stop.
    // Ignore SIGPIPE in case FFmpeg has already exited (broken pipe).
    if (session.stdinFd >= 0) {
        struct sigaction sa_old{}, sa_ign{};
        sa_ign.sa_handler = SIG_IGN;
        sigemptyset(&sa_ign.sa_mask);
        sigaction(SIGPIPE, &sa_ign, &sa_old);

        const char quit = 'q';
        ssize_t written = write(session.stdinFd, &quit, 1);
        (void)written; // may fail with EPIPE if FFmpeg already exited

        sigaction(SIGPIPE, &sa_old, nullptr);

        close(session.stdinFd);
        session.stdinFd = -1;
    }

    // Wait for process to exit (up to 10 seconds)
    int status = 0;
    int waitAttempts = 100; // 100 x 100ms = 10s
    pid_t wpid = 0;

    while (waitAttempts-- > 0) {
        wpid = waitpid(static_cast<pid_t>(session.pid), &status, WNOHANG);
        if (wpid != 0) break;
        usleep(100000); // 100ms
    }

    if (wpid == 0) {
        // Still running — force kill
        kill(static_cast<pid_t>(session.pid), SIGKILL);
        waitpid(static_cast<pid_t>(session.pid), &status, 0);
        result.errorMessage = "FFmpeg did not exit in time, force killed";
    }

    session.pid = -1;

    // Check if output file exists
    if (fileExists(result.outputPath)) {
        result.success = true;
    } else {
        if (result.errorMessage.empty()) {
            result.errorMessage = "Output file not found: " + result.outputPath;
        }
    }

    return result;
}

bool ScreenRecordingUtils_Mac::pauseRecording(const RecordingSession& session)
{
    if (!session.isValid()) return false;
    return kill(static_cast<pid_t>(session.pid), SIGSTOP) == 0;
}

bool ScreenRecordingUtils_Mac::resumeRecording(const RecordingSession& session)
{
    if (!session.isValid()) return false;
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
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty()) {
        return false;
    }

    // Build palette + GIF two-pass command as a single filter_complex
    std::string filterComplex = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    pid_t pid = fork();
    if (pid < 0) return false;

    if (pid == 0) {
        // Redirect output
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }

        execlp(ffmpegPath.c_str(), ffmpegPath.c_str(),
               "-y", "-i", inputPath.c_str(),
               "-filter_complex", filterComplex.c_str(),
               outputPath.c_str(),
               nullptr);
        _exit(127);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    return WIFEXITED(status) && WEXITSTATUS(status) == 0 && fileExists(outputPath);
}

} // namespace ucf::utilities::screenrecording

#endif // __APPLE__
