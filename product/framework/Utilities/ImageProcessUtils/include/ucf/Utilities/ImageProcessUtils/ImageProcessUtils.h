#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities::imageprocess {

// ============================================================================
// Data Types
// ============================================================================

/**
 * @brief Raw image data in RGBA format for cross-layer exchange
 *
 * Unlike CaptureImage (BGRA, platform-native), ImageData uses RGBA byte order
 * which is the standard format for image processing and encoding.
 */
struct Utilities_EXPORT ImageData {
    std::vector<uint8_t> pixels;   ///< Pixel data in RGBA byte order
    int width = 0;
    int height = 0;
    int bytesPerRow = 0;           ///< Defaults to width * 4

    bool isValid() const { return width > 0 && height > 0 && !pixels.empty(); }
};

struct Utilities_EXPORT Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

struct Utilities_EXPORT Point {
    int x = 0;
    int y = 0;
};

struct Utilities_EXPORT PointF {
    double x = 0.0;
    double y = 0.0;
};

struct Utilities_EXPORT Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

// ============================================================================
// Annotation Types
// ============================================================================

enum class AnnotationType {
    Rectangle,      ///< Unfilled rectangle outline
    FilledRect,     ///< Filled rectangle (for highlight/dim)
    Ellipse,        ///< Unfilled ellipse outline
    Arrow,          ///< Arrow line from start to end
    Line,           ///< Straight line
    FreehandLine,   ///< Polyline (freehand drawing)
    Text,           ///< Text label
    Mosaic          ///< Pixelated/mosaic region (privacy)
};

struct Utilities_EXPORT Annotation {
    AnnotationType type = AnnotationType::Rectangle;
    Color color = {255, 0, 0, 255};
    int thickness = 2;                      ///< Line thickness in pixels

    // Geometry — used according to type:
    Rect rect;                              ///< Rectangle, Ellipse, FilledRect, Mosaic
    Point startPoint;                       ///< Arrow, Line
    Point endPoint;                         ///< Arrow, Line
    std::vector<PointF> points;             ///< FreehandLine

    // Text-specific
    std::string text;
    int fontSize = 16;

    // Mosaic-specific
    int mosaicBlockSize = 10;               ///< Pixel block size for mosaic
};

// ============================================================================
// Encoding Result
// ============================================================================

struct Utilities_EXPORT EncodeResult {
    bool success = false;
    std::vector<uint8_t> data;              ///< Encoded image bytes (PNG/JPEG)
    std::string errorMessage;

    bool isValid() const { return success && !data.empty(); }
};

// ============================================================================
// ImageProcessUtils
// ============================================================================

/**
 * @brief Image processing utility using OpenCV internally
 *
 * Provides annotation drawing, format conversion, encoding, and region
 * extraction — all without Qt dependency. Uses OpenCV for internal processing;
 * external API exposes only plain C++ types (ImageData, Annotation, etc.).
 */
class Utilities_EXPORT ImageProcessUtils final {
public:
    // === Format Conversion ===

    /** Convert BGRA pixel data to RGBA ImageData */
    static ImageData bgraToRgba(const std::vector<uint8_t>& bgraPixels,
                                int width, int height, int bytesPerRow = 0);

    /** Convert RGBA ImageData to BGRA pixel data */
    static std::vector<uint8_t> rgbaToBgra(const ImageData& image);

    // === Region Extraction ===

    /** Crop a rectangular region from an image */
    static ImageData cropRegion(const ImageData& source, const Rect& region);

    // === Annotation Drawing ===

    /**
     * @brief Draw a single annotation onto the image (modified in-place)
     * @return true on success
     */
    static bool drawAnnotation(ImageData& image, const Annotation& annotation);

    /**
     * @brief Draw multiple annotations onto the image (modified in-place)
     * @return Number of successfully drawn annotations
     */
    static int drawAnnotations(ImageData& image,
                               const std::vector<Annotation>& annotations);

    // === Encoding ===

    /** Encode image to PNG format */
    static EncodeResult encodePng(const ImageData& image);

    /** Encode image to JPEG format */
    static EncodeResult encodeJpeg(const ImageData& image, int quality = 90);

    // === File I/O ===

    /** Save image to file (format detected from extension: .png, .jpg, .bmp) */
    static bool saveToFile(const ImageData& image, const std::string& filePath);

    // === Base64 ===

    /** Encode image as PNG, then Base64-encode the result */
    static std::string toBase64Png(const ImageData& image);

    // === Resize ===

    /** Resize image to specified dimensions */
    static ImageData resize(const ImageData& source, int newWidth, int newHeight);
};

} // namespace ucf::utilities::imageprocess
