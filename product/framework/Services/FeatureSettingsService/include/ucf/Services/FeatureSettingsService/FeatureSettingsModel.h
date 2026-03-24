#pragma once

#include <string>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service::model {

struct SERVICE_EXPORT ScreenshotFeatureSettings {
    std::string outputDirectory;
    std::string imageFormat = "png";     ///< png / jpg / bmp
    int jpegQuality = 90;               ///< 1-100 (JPEG only)
    int captureDelay = 0;               ///< Delay in seconds before capture
    bool addTimestamp = false;           ///< Overlay timestamp watermark
};

} // namespace ucf::service::model
