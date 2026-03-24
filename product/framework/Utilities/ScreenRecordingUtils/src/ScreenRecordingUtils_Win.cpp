#include "ScreenRecordingUtils_Win.h"

#ifdef _WIN32

#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

namespace ucf::utilities::screenrecording {

std::string ScreenRecordingUtils_Win::findFFmpegPath(const std::string& appDir)
{
    std::vector<std::string> candidates = {
        appDir + "/ffmpeg.exe",
        appDir + "/ffmpeg/ffmpeg.exe"
    };

    for (const auto& candidate : candidates) {
        std::error_code ec;
        auto canonical = fs::canonical(candidate, ec);
        if (!ec && fs::is_regular_file(canonical, ec)) {
            return canonical.string();
        }
    }

    // Fallback: search PATH via 'where'
    // TODO: Implement via CreateProcess or _popen
    return {};
}

RecordingSession ScreenRecordingUtils_Win::startRecording(const RecordingConfig& config)
{
    // TODO: Implement via CreateProcess with pipe for stdin
    (void)config;
    return {};
}

RecordingResult ScreenRecordingUtils_Win::stopRecording(RecordingSession& session)
{
    // TODO: Write 'q' to stdin pipe, WaitForSingleObject
    (void)session;
    return {false, {}, "Windows recording not yet implemented"};
}

bool ScreenRecordingUtils_Win::pauseRecording(const RecordingSession& session)
{
    // TODO: SuspendThread
    (void)session;
    return false;
}

bool ScreenRecordingUtils_Win::resumeRecording(const RecordingSession& session)
{
    // TODO: ResumeThread
    (void)session;
    return false;
}

bool ScreenRecordingUtils_Win::convertToGif(const std::string& ffmpegPath,
                                            const std::string& inputPath,
                                            const std::string& outputPath,
                                            int fps)
{
    // TODO: Implement via CreateProcess
    (void)ffmpegPath; (void)inputPath; (void)outputPath; (void)fps;
    return false;
}

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
