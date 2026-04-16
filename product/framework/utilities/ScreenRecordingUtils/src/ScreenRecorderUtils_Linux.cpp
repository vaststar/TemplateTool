#include "ScreenRecorder_Linux.h"

#ifdef __linux__

#include "LoggerDefine.h"

#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>

namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers
// ============================================================================

/// Search the PATH environment variable for an executable by name.
static std::string findInPath(const std::string& name)
{
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv)
    {
        return {};
    }

    std::istringstream stream(pathEnv);
    std::string dir;
    while (std::getline(stream, dir, ':'))
    {
        if (dir.empty())
        {
            continue;
        }
        auto candidate = std::filesystem::path(dir) / name;
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
    return {};
}

// ============================================================================
// FFmpeg discovery
// ============================================================================

std::string ScreenRecorder_Linux::findFFmpegPath(const std::string& appDir)
{
    std::vector<std::string> candidates = {
        appDir + "/ffmpeg",
        appDir + "/../lib/ffmpeg",
        "/usr/bin/ffmpeg",
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
// Audio Device Enumeration
// ============================================================================

std::vector<AudioDeviceInfo> ScreenRecorder_Linux::enumerateAudioDevices()
{
    std::vector<AudioDeviceInfo> devices;

    // Enumerate PulseAudio sources (microphones) and monitors (loopback) via pactl
    auto parseDevices = [&](bool isInput) {
        int pfd[2];
        if (pipe(pfd) != 0) return;

        pid_t pid = fork();
        if (pid < 0)
        {
            close(pfd[0]);
            close(pfd[1]);
            return;
        }
        if (pid == 0)
        {
            close(pfd[0]);
            dup2(pfd[1], STDOUT_FILENO);
            close(pfd[1]);
            int devNull = open("/dev/null", O_WRONLY);
            if (devNull >= 0) { dup2(devNull, STDERR_FILENO); close(devNull); }

            if (isInput)
                execlp("pactl", "pactl", "list", "sources", "short", nullptr);
            else
                execlp("pactl", "pactl", "list", "sinks", "short", nullptr);
            _exit(1);
        }
        close(pfd[1]);

        // Read output
        std::string output;
        char buf[1024];
        ssize_t n;
        while ((n = read(pfd[0], buf, sizeof(buf) - 1)) > 0)
        {
            buf[n] = '\0';
            output += buf;
        }
        close(pfd[0]);
        waitpid(pid, nullptr, 0);

        // Parse tab-separated lines: index\tname\tmodule\tsample_spec\tstate
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line))
        {
            if (line.empty()) continue;
            auto tabPos = line.find('\t');
            if (tabPos == std::string::npos) continue;
            auto nameStart = tabPos + 1;
            auto nameEnd = line.find('\t', nameStart);
            if (nameEnd == std::string::npos) nameEnd = line.size();

            std::string name = line.substr(nameStart, nameEnd - nameStart);
            if (name.empty()) continue;

            // For input sources, skip monitor sources (they are for loopback)
            if (isInput && name.find(".monitor") != std::string::npos) continue;
            // For output "loopback", show monitor sources
            if (!isInput && name.find(".monitor") == std::string::npos) continue;

            AudioDeviceType devType = isInput ? AudioDeviceType::Microphone : AudioDeviceType::LoopbackCapture;
            devices.push_back({name, name, isInput, devType});
        }
    };

    parseDevices(true);    // Microphones
    parseDevices(false);   // Monitor/loopback

    return devices;
}

// ============================================================================
// GIF Conversion
// ============================================================================

bool ScreenRecorder_Linux::convertToGif(const std::string& ffmpegPath,
                                              const std::string& inputPath,
                                              const std::string& outputPath,
                                              int fps)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
    {
        SRU_LOG_ERROR("convertToGif: empty path argument");
        return false;
    }

    std::string filterComplex = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    ucf::utilities::ProcessBridgeConfig config;
    config.executablePath = ffmpegPath;
    config.arguments = {"-y", "-i", inputPath, "-filter_complex", filterComplex, outputPath};
    config.stopTimeoutMs = 120000;

    SRU_LOG_INFO("convertToGif: " << inputPath << " -> " << outputPath);
    auto result = ucf::utilities::IProcessBridge::run(config);

    if (result.timedOut)
    {
        SRU_LOG_ERROR("convertToGif: FFmpeg timed out");
        return false;
    }

    std::error_code ec;
    auto sz = std::filesystem::file_size(outputPath, ec);
    if (result.exitCode != 0 || ec || sz == 0)
    {
        SRU_LOG_ERROR("convertToGif: failed or output empty");
        return false;
    }
    return true;
}

} // namespace ucf::utilities::screenrecording

#endif // __linux__
