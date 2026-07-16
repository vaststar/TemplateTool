#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities::screencapture {

/**
 * @brief Raw captured image data in platform-native BGRA format
 */
struct Utilities_EXPORT CaptureImage
{
    std::vector<uint8_t> pixels;   ///< Pixel data in BGRA byte order
    int width = 0;                 ///< Image width in physical pixels
    int height = 0;                ///< Image height in physical pixels
    int bytesPerRow = 0;           ///< Bytes per row (may include padding)
    int scaleFactor = 1;           ///< Display scale factor (Retina=2, normal=1)

    bool isValid() const
    {
        return width > 0 && height > 0 && !pixels.empty();
    }
};

/**
 * @brief Display/monitor information
 */
struct Utilities_EXPORT DisplayInfo
{
    int displayId = 0;
    std::string name;
    int x = 0;                     ///< X position in virtual desktop (logical pixels)
    int y = 0;                     ///< Y position in virtual desktop (logical pixels)
    int width = 0;                 ///< Logical width
    int height = 0;                ///< Logical height
    int physicalWidth = 0;         ///< Physical pixel width
    int physicalHeight = 0;        ///< Physical pixel height
    int scaleFactor = 1;           ///< DPI scale factor
    bool isPrimary = false;
};

/**
 * @brief Platform-specific screen capture utility
 *
 * Provides cross-platform screen capture via native platform APIs.
 * No Qt dependency — uses CoreGraphics (macOS), Win32 GDI (Windows), portal (Linux).
 */
class Utilities_EXPORT ScreenCaptureUtils final
{
public:
    // === Display Enumeration ===
    static std::vector<DisplayInfo> getDisplayList();
    static int getDisplayCount();
    static DisplayInfo getPrimaryDisplay();

    // === Screen Capture ===
    static CaptureImage captureDisplay(int displayIndex = 0);
    static CaptureImage captureAllDisplays();

    // === Permission (macOS screen recording permission) ===
    static bool hasScreenCapturePermission();
    static void requestScreenCapturePermission();
};

} // namespace ucf::utilities::screencapture
