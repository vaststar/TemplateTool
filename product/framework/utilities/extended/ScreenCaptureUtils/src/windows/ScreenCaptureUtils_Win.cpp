#include "ScreenCaptureUtils_Win.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <ShellScalingApi.h>  // GetDpiForMonitor
#include <dwmapi.h>           // DwmGetWindowAttribute

#include <algorithm>
#include <cstring>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shcore.lib")

namespace ucf::utilities::screencapture {

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

struct MonitorEnumContext
{
    std::vector<DisplayInfo> displays;
    int index = 0;
};

static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC /*hdc*/,
                                      LPRECT /*lpRect*/, LPARAM lParam)
{
    auto* ctx = reinterpret_cast<MonitorEnumContext*>(lParam);

    MONITORINFOEXW mi = {};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(hMonitor, &mi))
    {
        return TRUE;
    }

    DisplayInfo info;
    info.displayId = ctx->index;
    info.x = mi.rcMonitor.left;
    info.y = mi.rcMonitor.top;
    info.width = mi.rcMonitor.right - mi.rcMonitor.left;
    info.height = mi.rcMonitor.bottom - mi.rcMonitor.top;
    info.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;

    // DPI-aware physical size
    UINT dpiX = 96, dpiY = 96;
    if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
    {
        info.scaleFactor = static_cast<int>(dpiX / 96);
        if (info.scaleFactor < 1)
        {
            info.scaleFactor = 1;
        }
    }
    info.physicalWidth = info.width * info.scaleFactor;
    info.physicalHeight = info.height * info.scaleFactor;

    // Name from device name
    char nameBuf[64] = {};
    WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, nameBuf, sizeof(nameBuf), nullptr, nullptr);
    info.name = nameBuf;

    ctx->displays.push_back(std::move(info));
    ctx->index++;
    return TRUE;
}

/**
 * @brief Capture a specific region using BitBlt
 */
static CaptureImage captureRegion(int x, int y, int width, int height, int scaleFactor = 1)
{
    HDC hdcScreen = GetDC(nullptr);
    if (!hdcScreen)
    {
        return {};
    }

    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, x, y, SRCCOPY);

    SelectObject(hdcMem, hOld);

    // Extract pixel data (BGRA)
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(bi);
    bi.biWidth = width;
    bi.biHeight = -height; // top-down
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    int bytesPerRow = width * 4;
    CaptureImage result;
    result.width = width;
    result.height = height;
    result.bytesPerRow = bytesPerRow;
    result.scaleFactor = scaleFactor;
    result.pixels.resize(static_cast<size_t>(bytesPerRow) * height);

    GetDIBits(hdcMem, hBitmap, 0, height,
              result.pixels.data(),
              reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    return result;
}

// ---------------------------------------------------------------------------
// Display Enumeration
// ---------------------------------------------------------------------------

std::vector<DisplayInfo> ScreenCaptureUtils_Win::getDisplayList()
{
    MonitorEnumContext ctx;
    EnumDisplayMonitors(nullptr, nullptr, monitorEnumProc, reinterpret_cast<LPARAM>(&ctx));
    return ctx.displays;
}

int ScreenCaptureUtils_Win::getDisplayCount()
{
    return GetSystemMetrics(SM_CMONITORS);
}

DisplayInfo ScreenCaptureUtils_Win::getPrimaryDisplay()
{
    auto displays = getDisplayList();
    for (auto& d : displays)
    {
        if (d.isPrimary)
        {
            return d;
        }
    }
    if (!displays.empty())
    {
        return displays.front();
    }
    return {};
}

// ---------------------------------------------------------------------------
// Screen Capture
// ---------------------------------------------------------------------------

CaptureImage ScreenCaptureUtils_Win::captureDisplay(int displayIndex)
{
    auto displays = getDisplayList();
    if (displayIndex < 0 || static_cast<size_t>(displayIndex) >= displays.size())
    {
        return {};
    }

    const auto& d = displays[displayIndex];
    return captureRegion(d.x, d.y,
                         d.physicalWidth, d.physicalHeight,
                         d.scaleFactor);
}

CaptureImage ScreenCaptureUtils_Win::captureAllDisplays()
{
    // Virtual screen covers all monitors
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    if (w <= 0 || h <= 0)
    {
        return {};
    }

    return captureRegion(x, y, w, h);
}

// ---------------------------------------------------------------------------
// Permission — Windows always has screen capture access
// ---------------------------------------------------------------------------

bool ScreenCaptureUtils_Win::hasScreenCapturePermission()
{
    return true;
}

void ScreenCaptureUtils_Win::requestScreenCapturePermission()
{
    // No-op on Windows
}

} // namespace ucf::utilities::screencapture

#endif // _WIN32
