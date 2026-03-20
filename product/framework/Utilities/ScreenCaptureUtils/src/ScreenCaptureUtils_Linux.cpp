#include "ScreenCaptureUtils_Linux.h"

#ifdef __linux__

// TODO: Implement using X11 (XGetImage/XShmGetImage) or PipeWire (Wayland).
//       For now, all methods return empty/default values.

namespace ucf::utilities::screencapture {

std::vector<DisplayInfo> ScreenCaptureUtils_Linux::getDisplayList()
{
    return {};
}

int ScreenCaptureUtils_Linux::getDisplayCount()
{
    return 0;
}

DisplayInfo ScreenCaptureUtils_Linux::getPrimaryDisplay()
{
    return {};
}

CaptureImage ScreenCaptureUtils_Linux::captureDisplay(int /*displayIndex*/)
{
    return {};
}

CaptureImage ScreenCaptureUtils_Linux::captureAllDisplays()
{
    return {};
}

std::vector<WindowInfo> ScreenCaptureUtils_Linux::getWindowList()
{
    return {};
}

CaptureImage ScreenCaptureUtils_Linux::captureWindow(int64_t /*windowId*/)
{
    return {};
}

bool ScreenCaptureUtils_Linux::hasScreenCapturePermission()
{
    return true; // No permission model on X11
}

void ScreenCaptureUtils_Linux::requestScreenCapturePermission()
{
    // No-op
}

} // namespace ucf::utilities::screencapture

#endif // __linux__
