#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <utility>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::viewModels::model {

// ============================================================================
// Screenshot Settings
// ============================================================================

struct COMMONHEAD_EXPORT ScreenshotSettings {
    std::string outputDirectory;
    std::string imageFormat = "png";     ///< png / jpg / bmp
    int jpegQuality = 90;               ///< 1-100 (JPEG only)
    int captureDelay = 0;               ///< Delay in seconds before capture
    bool addTimestamp = false;           ///< Overlay timestamp watermark
};

// ============================================================================
// Annotation Data
// ============================================================================

struct COMMONHEAD_EXPORT AnnotationData {
    int id = 0;
    std::string type;                    ///< "rectangle"/"ellipse"/"arrow"/"line"/"freehand"/"text"/"mosaic"

    // Bounding box (Rectangle, Ellipse, FilledRect, Text, Mosaic)
    int x = 0, y = 0, w = 0, h = 0;

    // Endpoints (Arrow, Line)
    int startX = 0, startY = 0;
    int endX = 0, endY = 0;

    // Polyline points (Freehand)
    std::vector<std::pair<double, double>> points;

    // Text content
    std::string text;

    // Color (RGBA)
    uint8_t r = 255, g = 0, b = 0, a = 255;

    // Style
    int thickness = 2;
    int fontSize = 16;
    int mosaicBlockSize = 10;
};

// ============================================================================
// Screenshot State
// ============================================================================

enum class ScreenshotState {
    Idle,           ///< No active screenshot
    Captured,       ///< Full screen captured, waiting for region selection
    Editing,        ///< User is editing annotations
    Saving          ///< Screenshot is being saved
};

// ============================================================================
// Display / Window Info (ViewModel-layer mirror of Utilities types)
// ============================================================================

struct COMMONHEAD_EXPORT DisplayInfoVM {
    int displayId = 0;
    std::string name;
    int x = 0, y = 0, width = 0, height = 0;
    int scaleFactor = 1;
    bool isPrimary = false;
};

struct COMMONHEAD_EXPORT WindowInfoVM {
    int64_t windowId = 0;
    std::string name;
    std::string ownerName;
};

} // namespace commonHead::viewModels::model
