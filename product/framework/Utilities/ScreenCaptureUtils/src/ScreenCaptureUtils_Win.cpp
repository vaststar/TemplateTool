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

struct MonitorEnumContext {
    std::vector<DisplayInfo> displays;
    int index = 0;
};

static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC /*hdc*/,
                                      LPRECT /*lpRect*/, LPARAM lParam)
{
    auto* ctx = reinterpret_cast<MonitorEnumContext*>(lParam);

    MONITORINFOEXW mi = {};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(hMonitor, &mi)) {
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
    if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
        info.scaleFactor = static_cast<int>(dpiX / 96);
        if (info.scaleFactor < 1) info.scaleFactor = 1;
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
    if (!hdcScreen) {
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
    for (auto& d : displays) {
        if (d.isPrimary) {
            return d;
        }
    }
    if (!displays.empty()) {
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
    if (displayIndex < 0 || static_cast<size_t>(displayIndex) >= displays.size()) {
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

    if (w <= 0 || h <= 0) {
        return {};
    }

    return captureRegion(x, y, w, h);
}

// ---------------------------------------------------------------------------
// Window List / Capture
// ---------------------------------------------------------------------------

struct WindowEnumContext {
    std::vector<WindowInfo> windows;
};

static BOOL CALLBACK windowEnumProc(HWND hwnd, LPARAM lParam)
{
    auto* ctx = reinterpret_cast<WindowEnumContext*>(lParam);

    if (!IsWindowVisible(hwnd)) {
        return TRUE;
    }

    // Skip zero-size windows
    RECT rect = {};
    if (DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS,
                               &rect, sizeof(rect)) != S_OK) {
        GetWindowRect(hwnd, &rect);
    }

    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    if (w <= 0 || h <= 0) {
        return TRUE;
    }

    WindowInfo info;
    info.windowId = reinterpret_cast<int64_t>(hwnd);
    info.x = rect.left;
    info.y = rect.top;
    info.width = w;
    info.height = h;
    info.isOnScreen = true;
    info.isMinimized = IsIconic(hwnd) != 0;

    // Window title
    wchar_t titleW[256] = {};
    GetWindowTextW(hwnd, titleW, 256);
    char titleUtf8[512] = {};
    WideCharToMultiByte(CP_UTF8, 0, titleW, -1, titleUtf8, sizeof(titleUtf8), nullptr, nullptr);
    info.name = titleUtf8;

    // Owner process name (simplified — use process name)
    // For now, leave ownerName empty; can be extended with process enumeration
    info.ownerName = "";

    if (!info.name.empty()) {
        ctx->windows.push_back(std::move(info));
    }

    return TRUE;
}

std::vector<WindowInfo> ScreenCaptureUtils_Win::getWindowList()
{
    WindowEnumContext ctx;
    EnumWindows(windowEnumProc, reinterpret_cast<LPARAM>(&ctx));
    return ctx.windows;
}

CaptureImage ScreenCaptureUtils_Win::captureWindow(int64_t windowId)
{
    HWND hwnd = reinterpret_cast<HWND>(windowId);
    if (!IsWindow(hwnd)) {
        return {};
    }

    RECT rect = {};
    if (DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS,
                               &rect, sizeof(rect)) != S_OK) {
        GetWindowRect(hwnd, &rect);
    }

    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    if (w <= 0 || h <= 0) {
        return {};
    }

    // Use PrintWindow for better capture of off-screen/composited windows
    HDC hdcMem = CreateCompatibleDC(nullptr);
    HDC hdcScreen = GetDC(nullptr);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, w, h);
    HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);

    // PW_RENDERFULLCONTENT = 0x00000002 (Windows 8.1+)
    PrintWindow(hwnd, hdcMem, 0x00000002);

    SelectObject(hdcMem, hOld);

    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(bi);
    bi.biWidth = w;
    bi.biHeight = -h;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    int bytesPerRow = w * 4;
    CaptureImage result;
    result.width = w;
    result.height = h;
    result.bytesPerRow = bytesPerRow;
    result.scaleFactor = 1;
    result.pixels.resize(static_cast<size_t>(bytesPerRow) * h);

    GetDIBits(hdcMem, hBitmap, 0, h,
              result.pixels.data(),
              reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    return result;
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
