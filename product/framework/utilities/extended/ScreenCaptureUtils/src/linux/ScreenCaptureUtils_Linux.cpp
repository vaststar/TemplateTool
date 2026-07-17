#include "ScreenCaptureUtils_Linux.h"

#ifdef __linux__

#include "PortalFrameGrabber_Linux.h"

#include <sstream>
#include <regex>

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>

namespace ucf::utilities::screencapture {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Run a process and capture its stdout via ProcessBridge (no shell, so there
/// is no command-injection surface). Bare command names are resolved on PATH.
static std::string runCapture(const std::string& exe,
                              const std::vector<std::string>& args,
                              int timeoutMs = 15000)
{
    ucf::utilities::ProcessBridgeConfig config;
    config.executablePath = exe;
    config.arguments = args;
    config.stopTimeoutMs = timeoutMs;
    auto result = ucf::utilities::IProcessBridge::run(config);
    if (result.timedOut || result.exitCode != 0)
    {
        return {};
    }
    return result.stdoutData;
}

// ---------------------------------------------------------------------------
// Display enumeration — parse xrandr or wlr-randr
// ---------------------------------------------------------------------------

static std::vector<DisplayInfo> parseDisplays()
{
    std::vector<DisplayInfo> displays;

    // Try wlr-randr first (Wayland), then xrandr (X11)
    std::string output = runCapture("wlr-randr", {});
    if (output.empty())
    {
        output = runCapture("xrandr", {"--current"});
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

    // Grab the whole desktop once via the portal, then crop the requested
    // monitor out of it in memory (no second screenshot, no external tools).
    PortalFrame frame = grabPortalFrameBGRA();
    if (!frame.image.isValid())
    {
        return {};
    }

    const auto& d = displays[displayIndex];

    // Convert the display's virtual-desktop coordinates into offsets inside the
    // captured frame (the frame's origin is frame.regionX/Y).
    int cropX = d.x - frame.regionX;
    int cropY = d.y - frame.regionY;
    int cropW = d.width;
    int cropH = d.height;

    // Clamp to the captured frame bounds.
    if (cropX < 0) { cropW += cropX; cropX = 0; }
    if (cropY < 0) { cropH += cropY; cropY = 0; }
    if (cropX + cropW > frame.image.width)  { cropW = frame.image.width - cropX; }
    if (cropY + cropH > frame.image.height) { cropH = frame.image.height - cropY; }

    if (cropW <= 0 || cropH <= 0)
    {
        // Geometry did not line up (e.g. single-stream desktop) — return full frame.
        return frame.image;
    }

    // If the crop covers the entire frame, skip the copy.
    if (cropX == 0 && cropY == 0 &&
        cropW == frame.image.width && cropH == frame.image.height)
    {
        return frame.image;
    }

    CaptureImage cropped;
    cropped.width = cropW;
    cropped.height = cropH;
    cropped.bytesPerRow = cropW * 4;
    cropped.scaleFactor = frame.image.scaleFactor;
    cropped.pixels.resize(static_cast<size_t>(cropped.bytesPerRow) * cropH);

    for (int row = 0; row < cropH; ++row)
    {
        const uint8_t* src = frame.image.pixels.data()
            + static_cast<size_t>(cropY + row) * frame.image.bytesPerRow
            + static_cast<size_t>(cropX) * 4;
        uint8_t* dst = cropped.pixels.data()
            + static_cast<size_t>(row) * cropped.bytesPerRow;
        std::memcpy(dst, src, static_cast<size_t>(cropW) * 4);
    }

    return cropped;
}

CaptureImage ScreenCaptureUtils_Linux::captureAllDisplays()
{
    return grabPortalFrameBGRA().image;
}

bool ScreenCaptureUtils_Linux::hasScreenCapturePermission()
{
    // A stored restore token means the portal previously granted access and a
    // subsequent grab will proceed without a permission dialog.
    return portalHasStoredToken();
}

void ScreenCaptureUtils_Linux::requestScreenCapturePermission()
{
    // No-op — the xdg-desktop-portal ScreenCast dialog is shown automatically
    // on the first capture, and the restore token silences later prompts.
}

} // namespace ucf::utilities::screencapture

#endif // __linux__
