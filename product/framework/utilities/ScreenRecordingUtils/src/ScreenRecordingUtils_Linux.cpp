#include "ScreenRecordingUtils_Linux.h"

#ifdef __linux__

#include <filesystem>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

namespace fs = std::filesystem;

namespace ucf::utilities::screenrecording {

static bool fileExists(const std::string& path)
{
    struct stat st{};
    return (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode));
}

static std::string resolvePath(const std::string& path)
{
    char resolved[PATH_MAX];
    if (realpath(path.c_str(), resolved)) {
        return std::string(resolved);
    }
    return {};
}

std::string ScreenRecordingUtils_Linux::findFFmpegPath(const std::string& appDir)
{
    std::vector<std::string> candidates = {
        appDir + "/ffmpeg",
        "/usr/bin/ffmpeg",
        "/usr/local/bin/ffmpeg"
    };

    for (const auto& candidate : candidates) {
        std::string resolved = resolvePath(candidate);
        if (!resolved.empty() && fileExists(resolved)) {
            return resolved;
        }
    }

    return {};
}

RecordingSession ScreenRecordingUtils_Linux::startRecording(const RecordingConfig& config)
{
    // TODO: Implement via fork/exec with x11grab or pipewire
    (void)config;
    return {};
}

RecordingResult ScreenRecordingUtils_Linux::stopRecording(RecordingSession& session)
{
    (void)session;
    return {false, {}, "Linux recording not yet implemented"};
}

bool ScreenRecordingUtils_Linux::pauseRecording(const RecordingSession& session)
{
    if (!session.isValid()) return false;
    return kill(static_cast<pid_t>(session.pid), SIGSTOP) == 0;
}

bool ScreenRecordingUtils_Linux::resumeRecording(const RecordingSession& session)
{
    if (!session.isValid()) return false;
    return kill(static_cast<pid_t>(session.pid), SIGCONT) == 0;
}

bool ScreenRecordingUtils_Linux::convertToGif(const std::string& ffmpegPath,
                                              const std::string& inputPath,
                                              const std::string& outputPath,
                                              int fps)
{
    (void)ffmpegPath; (void)inputPath; (void)outputPath; (void)fps;
    return false;
}

} // namespace ucf::utilities::screenrecording

#endif // __linux__
