#include <ucf/Utilities/FFmpegUtils/Cli/FFmpegLocator.h>

#include <filesystem>
#include <mutex>
#include <unordered_map>

#include "FFmpegLocatorPlatform.h"
#include "FFmpegLogger.h"

namespace ucf::utilities::ffmpeg {

namespace {

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

// Cache resolved executables keyed by base name ("ffmpeg" / "ffprobe"). Only
// successful (non-empty) results are cached, so installing FFmpeg after a failed
// probe is still picked up on retry. The cache lives inside this function, so no
// module-level state leaks into the translation unit.
std::string locateCached(const std::string& base)
{
    static std::mutex mutex;
    static std::unordered_map<std::string, std::string> cache;

    std::lock_guard<std::mutex> lock(mutex);
    if (auto it = cache.find(base); it != cache.end())
    {
        return it->second;
    }
    std::string resolved = probeCandidates(base);
    if (!resolved.empty())
    {
        FF_LOG_INFO(base << " auto-discovered at: " << resolved);
        cache.emplace(base, resolved);
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
    return locateCached("ffmpeg");
}

std::string FFmpegLocator::ffprobePath()
{
    return locateCached("ffprobe");
}

bool FFmpegLocator::isAvailable()
{
    return !ffmpegPath().empty();
}

} // namespace ucf::utilities::ffmpeg
