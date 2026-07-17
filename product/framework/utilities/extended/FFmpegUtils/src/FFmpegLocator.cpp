#include <ucf/Utilities/FFmpegUtils/FFmpegLocator.h>

#include <filesystem>
#include <mutex>

#include "FFmpegLocatorPlatform.h"
#include "FFmpegLogger.h"

namespace ucf::utilities::ffmpeg {

namespace {

// Cache resolved executables. Only successful (non-empty) results are cached so
// that installing FFmpeg after a failed probe is still picked up on retry.
std::mutex g_cacheMutex;
std::string g_ffmpegCache;
std::string g_ffprobeCache;

std::string probeCandidates(const std::string& base)
{
    const std::string exeName = detail::executableName(base);
    const std::string libDir = detail::libraryDirectory();

    for (const std::string& candidate : detail::candidatePaths(exeName, libDir))
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec) &&
            detail::isExecutableFile(canonical.string()))
        {
            return canonical.string();
        }
    }
    return detail::findInPath(exeName);
}

std::string locateCached(const std::string& base, std::string& cache)
{
    std::lock_guard<std::mutex> lock(g_cacheMutex);
    if (!cache.empty())
    {
        return cache;
    }
    std::string resolved = probeCandidates(base);
    if (!resolved.empty())
    {
        FF_LOG_INFO(base << " auto-discovered at: " << resolved);
        cache = resolved;
    }
    else
    {
        FF_LOG_WARN(base << " not found in any known location");
    }
    return resolved;
}

} // namespace

std::string FFmpegLocator::ffmpegPath()
{
    return locateCached("ffmpeg", g_ffmpegCache);
}

std::string FFmpegLocator::ffprobePath()
{
    return locateCached("ffprobe", g_ffprobeCache);
}

std::string FFmpegLocator::ffmpegPath(const std::string& appDir)
{
    const std::string exeName = detail::executableName("ffmpeg");
    for (const std::string& candidate : detail::candidatePathsForAppDir(exeName, appDir))
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec))
        {
            return canonical.string();
        }
    }
    return detail::findInPath(exeName);
}

bool FFmpegLocator::isAvailable()
{
    return !ffmpegPath().empty();
}

} // namespace ucf::utilities::ffmpeg
