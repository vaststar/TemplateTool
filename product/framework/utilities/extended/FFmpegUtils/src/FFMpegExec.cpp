#include <ucf/Utilities/FFmpegUtils/FFMpegExec.h>

#include <algorithm>
#include <filesystem>

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>
#include <ucf/Utilities/ProcessBridgeUtils/ProcessBridgeConfig.h>

#include <ucf/Utilities/FFmpegUtils/FFmpegLocator.h>
#include "FFmpegLogger.h"

namespace ucf::utilities::ffmpeg {

namespace {

std::string resolveFfmpeg(const std::string& explicitPath)
{
    return explicitPath.empty() ? FFmpegLocator::ffmpegPath() : explicitPath;
}

std::string resolveFfprobe(const std::string& explicitPath)
{
    return explicitPath.empty() ? FFmpegLocator::ffprobePath() : explicitPath;
}

} // namespace

bool FFMpegExec::convertToGif(const std::string& inputPath,
                              const std::string& outputPath,
                              int fps,
                              const std::string& ffmpegPath)
{
    const std::string ffmpeg = resolveFfmpeg(ffmpegPath);
    if (ffmpeg.empty() || inputPath.empty() || outputPath.empty())
    {
        FF_LOG_ERROR("convertToGif: missing ffmpeg/input/output path");
        return false;
    }

    fps = std::max(1, fps);
    const std::string filter = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    ProcessBridgeConfig config;
    config.executablePath = ffmpeg;
    config.arguments = {"-y", "-i", inputPath, "-filter_complex", filter, outputPath};
    config.stopTimeoutMs = 120000;

    auto result = IProcessBridge::run(config);
    if (result.timedOut)
    {
        FF_LOG_ERROR("convertToGif: timed out");
        return false;
    }

    std::error_code ec;
    return result.exitCode == 0 && std::filesystem::is_regular_file(outputPath, ec);
}

bool FFMpegExec::extractThumbnail(const std::string& inputPath,
                                  const std::string& outputPath,
                                  double timeSeconds,
                                  int maxWidth,
                                  int maxHeight,
                                  const std::string& ffmpegPath)
{
    const std::string ffmpeg = resolveFfmpeg(ffmpegPath);
    if (ffmpeg.empty() || inputPath.empty() || outputPath.empty())
    {
        FF_LOG_ERROR("extractThumbnail: missing ffmpeg/input/output path");
        return false;
    }

    maxWidth = std::max(1, maxWidth);
    maxHeight = std::max(1, maxHeight);
    timeSeconds = std::max(0.0, timeSeconds);

    std::error_code ec;
    auto output = std::filesystem::path(outputPath);
    if (!output.parent_path().empty())
    {
        std::filesystem::create_directories(output.parent_path(), ec);
        if (ec)
        {
            FF_LOG_ERROR("extractThumbnail: cannot create output directory");
            return false;
        }
    }

    const std::string scaleFilter = "scale=w=" + std::to_string(maxWidth)
        + ":h=" + std::to_string(maxHeight)
        + ":force_original_aspect_ratio=decrease";

    ProcessBridgeConfig config;
    config.executablePath = ffmpeg;
    config.arguments = {
        "-y",
        "-ss", std::to_string(timeSeconds),
        "-i", inputPath,
        "-frames:v", "1",
        "-vf", scaleFilter,
        outputPath};
    config.stopTimeoutMs = 30000;

    auto result = IProcessBridge::run(config);
    if (result.timedOut)
    {
        FF_LOG_ERROR("extractThumbnail: timed out");
        return false;
    }

    return result.exitCode == 0 && std::filesystem::is_regular_file(outputPath, ec);
}

bool FFMpegExec::decodeToBgra(const std::string& imagePath,
                              RawImage& out,
                              const std::string& ffmpegPath,
                              const std::string& ffprobePath)
{
    out = RawImage{};

    const std::string ffmpeg = resolveFfmpeg(ffmpegPath);
    const std::string ffprobe = resolveFfprobe(ffprobePath);
    if (ffmpeg.empty() || ffprobe.empty() || imagePath.empty())
    {
        FF_LOG_ERROR("decodeToBgra: missing ffmpeg/ffprobe/image path");
        return false;
    }

    std::error_code ec;
    if (!std::filesystem::is_regular_file(imagePath, ec))
    {
        FF_LOG_ERROR("decodeToBgra: image not found: " << imagePath);
        return false;
    }

    // 1) Probe dimensions with ffprobe -> "WIDTHxHEIGHT".
    ProcessBridgeConfig probeConfig;
    probeConfig.executablePath = ffprobe;
    probeConfig.arguments = {
        "-v", "error",
        "-select_streams", "v:0",
        "-show_entries", "stream=width,height",
        "-of", "csv=s=x:p=0",
        imagePath};
    probeConfig.stopTimeoutMs = 15000;

    auto probe = IProcessBridge::run(probeConfig);
    if (probe.timedOut || probe.exitCode != 0)
    {
        FF_LOG_ERROR("decodeToBgra: ffprobe failed for " << imagePath);
        return false;
    }

    int width = 0;
    int height = 0;
    {
        std::string dims = probe.stdoutData;
        auto xpos = dims.find('x');
        if (xpos == std::string::npos)
        {
            FF_LOG_ERROR("decodeToBgra: unexpected ffprobe output: " << dims);
            return false;
        }
        try
        {
            width = std::stoi(dims.substr(0, xpos));
            height = std::stoi(dims.substr(xpos + 1));
        }
        catch (const std::exception&)
        {
            FF_LOG_ERROR("decodeToBgra: cannot parse dimensions: " << dims);
            return false;
        }
    }

    if (width <= 0 || height <= 0)
    {
        FF_LOG_ERROR("decodeToBgra: invalid dimensions " << width << "x" << height);
        return false;
    }

    // 2) Decode to raw top-down BGRA on stdout.
    ProcessBridgeConfig decodeConfig;
    decodeConfig.executablePath = ffmpeg;
    decodeConfig.arguments = {"-y", "-i", imagePath, "-f", "rawvideo", "-pix_fmt", "bgra", "-"};
    decodeConfig.stopTimeoutMs = 30000;

    auto decode = IProcessBridge::run(decodeConfig);
    if (decode.timedOut || decode.exitCode != 0)
    {
        FF_LOG_ERROR("decodeToBgra: ffmpeg decode failed for " << imagePath);
        return false;
    }

    const size_t expected = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    if (decode.stdoutData.size() < expected)
    {
        FF_LOG_ERROR("decodeToBgra: short pixel data: got " << decode.stdoutData.size()
                     << " expected " << expected);
        return false;
    }

    out.width = width;
    out.height = height;
    out.pixels.assign(decode.stdoutData.begin(), decode.stdoutData.begin() + expected);
    return true;
}

} // namespace ucf::utilities::ffmpeg
