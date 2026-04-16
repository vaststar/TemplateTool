#include "ScreenCaptureUtils_Linux.h"

#ifdef __linux__

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <memory>
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

/// Find a usable ffmpeg for image conversion
static std::string findFfmpeg()
{
    for (const char* p : {"/usr/bin/ffmpeg", "/usr/local/bin/ffmpeg"})
    {
        if (access(p, X_OK) == 0)
        {
            return p;
        }
    }
    // Check alongside the running binary
    std::string selfDir;
    char buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0)
    {
        buf[len] = '\0';
        selfDir = std::filesystem::path(buf).parent_path().string();
        std::string candidate = selfDir + "/ffmpeg";
        if (access(candidate.c_str(), X_OK) == 0)
        {
            return candidate;
        }
    }
    return {};
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

std::vector<WindowInfo> ScreenCaptureUtils_Linux::getWindowList()
{
    std::vector<WindowInfo> windows;

    // Method 1: wmctrl (if installed, works on X11/XWayland)
    std::string output = execCmd("wmctrl -l -G 2>/dev/null");
    if (!output.empty())
    {
        std::istringstream iss(output);
        std::string line;
        while (std::getline(iss, line))
        {
            std::istringstream ls(line);
            std::string idStr;
            int desktop, x, y, w, h;
            std::string hostname, title;
            if (!(ls >> idStr >> desktop >> x >> y >> w >> h >> hostname))
            {
                continue;
            }
            std::getline(ls, title);
            size_t start = title.find_first_not_of(' ');
            if (start != std::string::npos)
            {
                title = title.substr(start);
            }

            WindowInfo wi;
            wi.windowId = std::strtoll(idStr.c_str(), nullptr, 16);
            wi.name = title;
            wi.ownerName = hostname;
            wi.x = x; wi.y = y; wi.width = w; wi.height = h;
            wi.isOnScreen = true;
            wi.isMinimized = false;
            windows.push_back(wi);
        }
        return windows;
    }

    // Method 2: xprop + xwininfo via Xwayland
    // Get the client list from root window
    output = execCmd("DISPLAY=:0 xprop -root _NET_CLIENT_LIST 2>/dev/null");
    if (output.empty())
    {
        return windows;
    }

    // Parse: _NET_CLIENT_LIST(WINDOW): window id # 0x1200003, 0x1200047, ...
    auto hashPos = output.find('#');
    if (hashPos == std::string::npos)
    {
        return windows;
    }
    std::string idList = output.substr(hashPos + 1);

    // Extract each window ID
    std::regex idRe("0x[0-9a-fA-F]+");
    auto it = std::sregex_iterator(idList.begin(), idList.end(), idRe);
    for (; it != std::sregex_iterator(); ++it)
    {
        std::string idHex = (*it).str();
        long long wid = std::strtoll(idHex.c_str(), nullptr, 16);

        // Query window name
        std::string nameOut = execCmd("DISPLAY=:0 xprop -id " + idHex
                                      + " _NET_WM_NAME 2>/dev/null");
        std::string name;
        // Parse: _NET_WM_NAME(UTF8_STRING) = "Title"
        auto eqPos = nameOut.find('=');
        if (eqPos != std::string::npos)
        {
            name = nameOut.substr(eqPos + 1);
            // Trim quotes and whitespace
            size_t s = name.find('"');
            size_t e = name.rfind('"');
            if (s != std::string::npos && e != std::string::npos && e > s)
            {
                name = name.substr(s + 1, e - s - 1);
            }
        }
        if (name.empty())
        {
            continue; // skip unnamed
        }

        // Query window geometry
        std::string geomOut = execCmd("DISPLAY=:0 xwininfo -id " + idHex
                                      + " 2>/dev/null");
        int x = 0, y = 0, w = 0, h = 0;
        std::smatch gm;
        std::regex xRe(R"(Absolute upper-left X:\s+(\d+))");
        std::regex yRe(R"(Absolute upper-left Y:\s+(\d+))");
        std::regex wRe(R"(Width:\s+(\d+))");
        std::regex hRe(R"(Height:\s+(\d+))");
        if (std::regex_search(geomOut, gm, xRe))
        {
            x = std::stoi(gm[1].str());
        }
        if (std::regex_search(geomOut, gm, yRe))
        {
            y = std::stoi(gm[1].str());
        }
        if (std::regex_search(geomOut, gm, wRe))
        {
            w = std::stoi(gm[1].str());
        }
        if (std::regex_search(geomOut, gm, hRe))
        {
            h = std::stoi(gm[1].str());
        }

        WindowInfo wi;
        wi.windowId = wid;
        wi.name = name;
        wi.x = x; wi.y = y; wi.width = w; wi.height = h;
        wi.isOnScreen = true;
        wi.isMinimized = false;
        windows.push_back(wi);
    }
    return windows;
}

CaptureImage ScreenCaptureUtils_Linux::captureWindow(int64_t windowId)
{
    // gnome-screenshot can capture a specific window by id
    std::string tmpFile = "/tmp/ucf_winshot_" + std::to_string(getpid()) + ".png";
    std::string hexId = "0x" + ([&]{
        std::ostringstream oss;
        oss << std::hex << windowId;
        return oss.str();
    })();

    // Use xwd + ffmpeg to capture the window (works on Xwayland)
    std::string xwdFile = "/tmp/ucf_winshot_" + std::to_string(getpid()) + ".xwd";
    std::string cmd = "DISPLAY=:0 xwd -id " + hexId + " -out \"" + xwdFile + "\" 2>/dev/null";
    bool captured = (std::system(cmd.c_str()) == 0 && access(xwdFile.c_str(), F_OK) == 0);

    if (!captured)
    {
        // Fallback: try import (ImageMagick)
        cmd = "DISPLAY=:0 import -window " + hexId + " \"" + tmpFile + "\" 2>/dev/null";
        if (std::system(cmd.c_str()) != 0 || access(tmpFile.c_str(), F_OK) != 0)
        {
            return {};
        }
    }

    std::string ffmpeg = findFfmpeg();
    if (ffmpeg.empty())
    {
        unlink(tmpFile.c_str());
        return {};
    }

    // If we captured via xwd, convert xwd→png first
    if (access(xwdFile.c_str(), F_OK) == 0)
    {
        std::string cvtCmd = "\"" + ffmpeg + "\" -y -i \"" + xwdFile
                            + "\" \"" + tmpFile + "\" 2>/dev/null";
        std::system(cvtCmd.c_str());
        unlink(xwdFile.c_str());
    }

    auto img = pngToBgraImage(tmpFile, ffmpeg);
    unlink(tmpFile.c_str());
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
