#include <ucf/Utilities/ScreenCaptureUtils/ScreenCaptureUtils.h>

#if defined(_WIN32)
#include "ScreenCaptureUtils_Win.h"
#elif defined(__APPLE__)
#include "ScreenCaptureUtils_Mac.h"
#elif defined(__linux__)
#include "ScreenCaptureUtils_Linux.h"
#endif

namespace ucf::utilities::screencapture {

std::vector<DisplayInfo> ScreenCaptureUtils::getDisplayList()
{
#if defined(_WIN32)
    return ScreenCaptureUtils_Win::getDisplayList();
#elif defined(__APPLE__)
    return ScreenCaptureUtils_Mac::getDisplayList();
#elif defined(__linux__)
    return ScreenCaptureUtils_Linux::getDisplayList();
#else
    return {};
#endif
}

int ScreenCaptureUtils::getDisplayCount()
{
#if defined(_WIN32)
    return ScreenCaptureUtils_Win::getDisplayCount();
#elif defined(__APPLE__)
    return ScreenCaptureUtils_Mac::getDisplayCount();
#elif defined(__linux__)
    return ScreenCaptureUtils_Linux::getDisplayCount();
#else
    return 0;
#endif
}

DisplayInfo ScreenCaptureUtils::getPrimaryDisplay()
{
#if defined(_WIN32)
    return ScreenCaptureUtils_Win::getPrimaryDisplay();
#elif defined(__APPLE__)
    return ScreenCaptureUtils_Mac::getPrimaryDisplay();
#elif defined(__linux__)
    return ScreenCaptureUtils_Linux::getPrimaryDisplay();
#else
    return {};
#endif
}

CaptureImage ScreenCaptureUtils::captureDisplay(int displayIndex)
{
#if defined(_WIN32)
    return ScreenCaptureUtils_Win::captureDisplay(displayIndex);
#elif defined(__APPLE__)
    return ScreenCaptureUtils_Mac::captureDisplay(displayIndex);
#elif defined(__linux__)
    return ScreenCaptureUtils_Linux::captureDisplay(displayIndex);
#else
    (void)displayIndex;
    return {};
#endif
}

CaptureImage ScreenCaptureUtils::captureAllDisplays()
{
#if defined(_WIN32)
    return ScreenCaptureUtils_Win::captureAllDisplays();
#elif defined(__APPLE__)
    return ScreenCaptureUtils_Mac::captureAllDisplays();
#elif defined(__linux__)
    return ScreenCaptureUtils_Linux::captureAllDisplays();
#else
    return {};
#endif
}

std::vector<WindowInfo> ScreenCaptureUtils::getWindowList()
{
#if defined(_WIN32)
    return ScreenCaptureUtils_Win::getWindowList();
#elif defined(__APPLE__)
    return ScreenCaptureUtils_Mac::getWindowList();
#elif defined(__linux__)
    return ScreenCaptureUtils_Linux::getWindowList();
#else
    return {};
#endif
}

CaptureImage ScreenCaptureUtils::captureWindow(int64_t windowId)
{
#if defined(_WIN32)
    return ScreenCaptureUtils_Win::captureWindow(windowId);
#elif defined(__APPLE__)
    return ScreenCaptureUtils_Mac::captureWindow(windowId);
#elif defined(__linux__)
    return ScreenCaptureUtils_Linux::captureWindow(windowId);
#else
    (void)windowId;
    return {};
#endif
}

bool ScreenCaptureUtils::hasScreenCapturePermission()
{
#if defined(_WIN32)
    return ScreenCaptureUtils_Win::hasScreenCapturePermission();
#elif defined(__APPLE__)
    return ScreenCaptureUtils_Mac::hasScreenCapturePermission();
#elif defined(__linux__)
    return ScreenCaptureUtils_Linux::hasScreenCapturePermission();
#else
    return false;
#endif
}

void ScreenCaptureUtils::requestScreenCapturePermission()
{
#if defined(_WIN32)
    ScreenCaptureUtils_Win::requestScreenCapturePermission();
#elif defined(__APPLE__)
    ScreenCaptureUtils_Mac::requestScreenCapturePermission();
#elif defined(__linux__)
    ScreenCaptureUtils_Linux::requestScreenCapturePermission();
#endif
}

} // namespace ucf::utilities::screencapture
