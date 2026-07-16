#pragma once

#include <ucf/Utilities/ScreenCaptureUtils/ScreenCaptureUtils.h>

namespace ucf::utilities::screencapture {

class ScreenCaptureUtils_Mac final
{
public:
    static std::vector<DisplayInfo> getDisplayList();
    static int getDisplayCount();
    static DisplayInfo getPrimaryDisplay();

    static CaptureImage captureDisplay(int displayIndex);
    static CaptureImage captureAllDisplays();

    static bool hasScreenCapturePermission();
    static void requestScreenCapturePermission();
};

} // namespace ucf::utilities::screencapture
