#include "ScreenCaptureUtils_Linux.h"

#ifdef __linux__

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <memory>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/wait.h>

namespace ucf::utilities::screencapture {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Run a command and capture stdout
static std::string execCmd(const std::string& cmd)
{
    std::array<char, 4096> buf;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        return {};
    }
    while (fgets(buf.data(), static_cast<int>(buf.size()), pipe.get()))
    {
        result += buf.data();
    }
    return result;
}

/// Read a PNG file into BGRA CaptureImage using stb-style decode or raw convert.
/// We use `ffmpeg -i <png> -f rawvideo -pix_fmt bgra -` to convert PNG→BGRA.
static CaptureImage pngToBgraImage(const std::string& pngPath, const std::string& ffmpegPath)
{
    CaptureImage img;
    // Get image dimensions first via `file` or ffprobe
    std::string probeCmd = "file \"" + pngPath + "\"";
    std::string probeOut = execCmd(probeCmd);
    // Parse "PNG image data, 1920 x 1080, ..."
    std::regex dimRe(R"((\d+)\s*x\s*(\d+))");
    std::smatch m;
    if (!std::regex_search(probeOut, m, dimRe) || m.size() < 3)
    {
        return img;
    }

    img.width = std::stoi(m[1].str());
    img.height = std::stoi(m[2].str());
    img.bytesPerRow = img.width * 4;
    img.scaleFactor = 1;

    // Convert PNG to raw BGRA via ffmpeg
    std::string cmd = "\"" + ffmpegPath + "\" -y -i \"" + pngPath
                      + "\" -f rawvideo -pix_fmt bgra - 2>/dev/null";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        return {};
    }

    size_t totalBytes = static_cast<size_t>(img.width) * img.height * 4;
    img.pixels.resize(totalBytes);
    size_t offset = 0;
    while (offset < totalBytes)
    {
        size_t n = fread(img.pixels.data() + offset, 1, totalBytes - offset, pipe.get());
        if (n == 0)
        {
            break;
        }
        offset += n;
    }
    if (offset < totalBytes)
    {
        img.pixels.clear();
        img.width = img.height = 0;
    }
    return img;
}

/// Find an executable by scanning the PATH environment variable.
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
        if (dir.empty()) continue;
        auto candidate = std::filesystem::path(dir) / name;
        std::error_code ec;
        if (std::filesystem::is_regular_file(candidate, ec) && access(candidate.c_str(), X_OK) == 0)
        {
            auto canonical = std::filesystem::canonical(candidate, ec);
            if (!ec) return canonical.string();
        }
    }
    return {};
}

/// Directory of the shared library this code is linked into. Resolving relative
/// to the library (not /proc/self/exe) is what lets us find ffmpeg in the
/// installed layout, where the util lives in lib/ next to ffmpeg while the
/// executable sits in a separate bin/.
static std::string getLibraryDirectory()
{
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(&getLibraryDirectory), &info) == 0
        || info.dli_fname == nullptr)
    {
        return {};
    }

    std::string fullPath = info.dli_fname;
    if (!fullPath.empty() && fullPath[0] != '/')
    {
        char resolved[PATH_MAX] = {};
        if (realpath(fullPath.c_str(), resolved))
        {
            fullPath = resolved;
        }
    }

    auto lastSep = fullPath.rfind('/');
    return (lastSep != std::string::npos) ? fullPath.substr(0, lastSep) : ".";
}

/// Find a usable ffmpeg for image conversion.
static std::string findFfmpeg()
{
    std::string libDir = getLibraryDirectory();
    if (libDir.empty())
    {
        return findInPath("ffmpeg");
    }

    // Cover both the dev build (ffmpeg beside the binary/library) and the
    // installed layout (executable in bin/, ffmpeg in the sibling lib/).
    for (const auto& candidate : {
             libDir + "/ffmpeg",
             libDir + "/../bin/ffmpeg",
             libDir + "/../lib/ffmpeg",
             std::string("/usr/bin/ffmpeg"),
             std::string("/usr/local/bin/ffmpeg")})
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec) && access(canonical.c_str(), X_OK) == 0)
        {
            return canonical.string();
        }
    }
    return findInPath("ffmpeg");
}

/// Take a screenshot to a temp PNG file using available tools.
/// Returns the path to the PNG, or empty string on failure.
static std::string screenshotToTempPng()
{
    std::string tmpFile = "/tmp/ucf_screenshot_" + std::to_string(getpid()) + ".png";
    unlink(tmpFile.c_str()); // remove stale file

    // Try gnome-screenshot (works on both X11 and Wayland via GNOME Shell D-Bus)
    {
        std::string cmd = "gnome-screenshot -f \"" + tmpFile + "\" 2>/dev/null";
        if (std::system(cmd.c_str()) == 0 && access(tmpFile.c_str(), F_OK) == 0)
        {
            return tmpFile;
        }
    }

    // Try grim (Wayland-native, common on wlroots compositors)
    {
        std::string cmd = "grim \"" + tmpFile + "\" 2>/dev/null";
        if (std::system(cmd.c_str()) == 0 && access(tmpFile.c_str(), F_OK) == 0)
        {
            return tmpFile;
        }
    }

    // Fallback: capture a single frame via ffmpeg x11grab (works via Xwayland)
    {
        std::string ffmpeg = findFfmpeg();
        if (!ffmpeg.empty())
        {
            std::string cmd = "\"" + ffmpeg + "\" -y -f x11grab -framerate 1 "
                              "-i :0.0 -frames:v 1 \"" + tmpFile + "\" 2>/dev/null";
            if (std::system(cmd.c_str()) == 0 && access(tmpFile.c_str(), F_OK) == 0)
            {
                return tmpFile;
            }
        }
    }

    return {};
}

/// Take a screenshot of a specific region
static std::string screenshotRegionToTempPng(int x, int y, int w, int h)
{
    std::string tmpFile = "/tmp/ucf_screenshot_" + std::to_string(getpid()) + ".png";
    unlink(tmpFile.c_str());

    // Try grim with geometry (Wayland-native)
    {
        std::string geom = std::to_string(x) + "," + std::to_string(y)
                           + " " + std::to_string(w) + "x" + std::to_string(h);
        std::string cmd = "grim -g \"" + geom + "\" \"" + tmpFile + "\" 2>/dev/null";
        if (std::system(cmd.c_str()) == 0 && access(tmpFile.c_str(), F_OK) == 0)
        {
            return tmpFile;
        }
    }

    // Fallback: ffmpeg x11grab single frame with region
    {
        std::string ffmpeg = findFfmpeg();
        if (!ffmpeg.empty())
        {
            std::string cmd = "\"" + ffmpeg + "\" -y -f x11grab -framerate 1 "
                              "-video_size " + std::to_string(w) + "x" + std::to_string(h)
                              + " -i :0.0+" + std::to_string(x) + "," + std::to_string(y)
                              + " -frames:v 1 \"" + tmpFile + "\" 2>/dev/null";
            if (std::system(cmd.c_str()) == 0 && access(tmpFile.c_str(), F_OK) == 0)
            {
                return tmpFile;
            }
        }
    }

    return {};
}

// ---------------------------------------------------------------------------
// Display enumeration — parse xrandr or wlr-randr
// ---------------------------------------------------------------------------

static std::vector<DisplayInfo> parseDisplays()
{
    std::vector<DisplayInfo> displays;

    // Try wlr-randr first (Wayland), then xrandr (X11)
    std::string output = execCmd("wlr-randr 2>/dev/null");
    if (output.empty())
    {
        output = execCmd("xrandr --current 2>/dev/null");
    }
    if (output.empty())
    {
        return displays;
    }

    // Parse xrandr-style output:
    //   DP-1 connected primary 1920x1080+0+0 ...
    //   HDMI-1 connected 2560x1440+1920+0 ...
    std::regex connRe(R"(^(\S+)\s+connected\s*(primary)?\s*(\d+)x(\d+)\+(\d+)\+(\d+))",
                      std::regex::multiline);
    auto it = std::sregex_iterator(output.begin(), output.end(), connRe);
    int idx = 0;
    for (; it != std::sregex_iterator(); ++it, ++idx)
    {
        DisplayInfo d;
        d.displayId = idx;
        d.name = (*it)[1].str();
        d.isPrimary = ((*it)[2].length() > 0);
        d.width = d.physicalWidth = std::stoi((*it)[3].str());
        d.height = d.physicalHeight = std::stoi((*it)[4].str());
        d.x = std::stoi((*it)[5].str());
        d.y = std::stoi((*it)[6].str());
        d.scaleFactor = 1;
        displays.push_back(d);
    }

    return displays;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::vector<DisplayInfo> ScreenCaptureUtils_Linux::getDisplayList()
{
    return parseDisplays();
}

int ScreenCaptureUtils_Linux::getDisplayCount()
{
    return static_cast<int>(parseDisplays().size());
}

DisplayInfo ScreenCaptureUtils_Linux::getPrimaryDisplay()
{
    auto list = parseDisplays();
    for (const auto& d : list)
    {
        if (d.isPrimary)
        {
            return d;
        }
    }
    return list.empty() ? DisplayInfo{} : list[0];
}

CaptureImage ScreenCaptureUtils_Linux::captureDisplay(int displayIndex)
{
    auto displays = parseDisplays();
    if (displayIndex < 0 || displayIndex >= static_cast<int>(displays.size()))
    {
        return {};
    }

    const auto& d = displays[displayIndex];
    std::string png = screenshotRegionToTempPng(d.x, d.y, d.width, d.height);
    if (png.empty())
    {
        // Fallback: capture all and crop later (not implemented, just capture all)
        png = screenshotToTempPng();
    }
    if (png.empty())
    {
        return {};
    }

    std::string ffmpeg = findFfmpeg();
    if (ffmpeg.empty())
    {
        unlink(png.c_str());
        return {};
    }

    auto img = pngToBgraImage(png, ffmpeg);
    unlink(png.c_str());
    return img;
}

CaptureImage ScreenCaptureUtils_Linux::captureAllDisplays()
{
    std::string png = screenshotToTempPng();
    if (png.empty())
    {
        return {};
    }

    std::string ffmpeg = findFfmpeg();
    if (ffmpeg.empty())
    {
        unlink(png.c_str());
        return {};
    }

    auto img = pngToBgraImage(png, ffmpeg);
    unlink(png.c_str());
    return img;
}

bool ScreenCaptureUtils_Linux::hasScreenCapturePermission()
{
    return true; // Wayland portal handles permissions via user interaction
}

void ScreenCaptureUtils_Linux::requestScreenCapturePermission()
{
    // No-op — Wayland portal prompts the user automatically
}

} // namespace ucf::utilities::screencapture

#endif // __linux__
