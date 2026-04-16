#include <ucf/Utilities/ImageProcessUtils/ImageProcessUtils.h>
#include <ucf/Utilities/Base64Utils/Base64Utils.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>

namespace ucf::utilities::imageprocess {

// ============================================================================
// Internal helpers: ImageData <-> cv::Mat conversion
// ============================================================================

/**
 * @brief Wrap ImageData as a cv::Mat (RGBA) without copying pixel data.
 *        The ImageData must outlive the returned Mat.
 */
static cv::Mat imageDataToMat(ImageData& image)
{
    int rows = image.height;
    int cols = image.width;
    int step = image.bytesPerRow > 0 ? image.bytesPerRow : cols * 4;
    return cv::Mat(rows, cols, CV_8UC4, image.pixels.data(), step);
}

static cv::Mat imageDataToMatConst(const ImageData& image)
{
    int rows = image.height;
    int cols = image.width;
    int step = image.bytesPerRow > 0 ? image.bytesPerRow : cols * 4;
    return cv::Mat(rows, cols, CV_8UC4,
                   const_cast<uint8_t*>(image.pixels.data()), step);
}

/**
 * @brief Create an ImageData that owns a copy of the cv::Mat data (assumed RGBA, CV_8UC4).
 */
static ImageData matToImageData(const cv::Mat& mat)
{
    ImageData result;
    if (mat.empty() || mat.type() != CV_8UC4) {
        return result;
    }

    result.width = mat.cols;
    result.height = mat.rows;
    result.bytesPerRow = static_cast<int>(mat.step);

    // If the Mat is continuous, we can copy directly; otherwise row-by-row
    if (mat.isContinuous()) {
        result.pixels.assign(mat.data, mat.data + mat.total() * mat.elemSize());
    } else {
        result.pixels.resize(static_cast<size_t>(result.bytesPerRow) * result.height);
        for (int r = 0; r < mat.rows; ++r) {
            std::memcpy(result.pixels.data() + r * result.bytesPerRow,
                        mat.ptr(r),
                        result.bytesPerRow);
        }
    }

    return result;
}

/**
 * @brief Convert Color to cv::Scalar in RGBA order (matching our Mat channel order)
 */
static cv::Scalar colorToScalar(const Color& c)
{
    return cv::Scalar(c.r, c.g, c.b, c.a);
}

// ============================================================================
// Format Conversion
// ============================================================================

ImageData ImageProcessUtils::bgraToRgba(const std::vector<uint8_t>& bgraPixels,
                                         int width, int height, int bytesPerRow)
{
    if (bgraPixels.empty() || width <= 0 || height <= 0) {
        return {};
    }

    int srcStep = bytesPerRow > 0 ? bytesPerRow : width * 4;
    cv::Mat bgraMat(height, width, CV_8UC4,
                    const_cast<uint8_t*>(bgraPixels.data()), srcStep);

    cv::Mat rgbaMat;
    // BGRA → RGBA: swap channels 0,2
    cv::cvtColor(bgraMat, rgbaMat, cv::COLOR_BGRA2RGBA);

    return matToImageData(rgbaMat);
}

std::vector<uint8_t> ImageProcessUtils::rgbaToBgra(const ImageData& image)
{
    if (!image.isValid()) {
        return {};
    }

    cv::Mat rgbaMat = imageDataToMatConst(image);
    cv::Mat bgraMat;
    cv::cvtColor(rgbaMat, bgraMat, cv::COLOR_RGBA2BGRA);

    std::vector<uint8_t> result;
    if (bgraMat.isContinuous()) {
        result.assign(bgraMat.data, bgraMat.data + bgraMat.total() * bgraMat.elemSize());
    } else {
        result.resize(static_cast<size_t>(bgraMat.step) * bgraMat.rows);
        for (int r = 0; r < bgraMat.rows; ++r) {
            std::memcpy(result.data() + r * bgraMat.step,
                        bgraMat.ptr(r),
                        bgraMat.step);
        }
    }
    return result;
}

// ============================================================================
// Region Extraction
// ============================================================================

ImageData ImageProcessUtils::cropRegion(const ImageData& source, const Rect& region)
{
    if (!source.isValid()) {
        return {};
    }

    // Clamp region to image bounds
    int x1 = std::max(0, region.x);
    int y1 = std::max(0, region.y);
    int x2 = std::min(source.width, region.x + region.width);
    int y2 = std::min(source.height, region.y + region.height);

    if (x2 <= x1 || y2 <= y1) {
        return {};
    }

    cv::Mat srcMat = imageDataToMatConst(source);
    cv::Rect roi(x1, y1, x2 - x1, y2 - y1);
    cv::Mat cropped = srcMat(roi).clone();

    return matToImageData(cropped);
}

// ============================================================================
// Annotation Drawing
// ============================================================================

static void drawRectangle(cv::Mat& mat, const Annotation& ann)
{
    cv::Rect rect(ann.rect.x, ann.rect.y, ann.rect.width, ann.rect.height);
    cv::rectangle(mat, rect, colorToScalar(ann.color), ann.thickness, cv::LINE_AA);
}

static void drawFilledRect(cv::Mat& mat, const Annotation& ann)
{
    cv::Rect rect(ann.rect.x, ann.rect.y, ann.rect.width, ann.rect.height);
    cv::rectangle(mat, rect, colorToScalar(ann.color), cv::FILLED, cv::LINE_AA);
}

static void drawEllipse(cv::Mat& mat, const Annotation& ann)
{
    cv::Point center(ann.rect.x + ann.rect.width / 2,
                     ann.rect.y + ann.rect.height / 2);
    cv::Size axes(ann.rect.width / 2, ann.rect.height / 2);
    cv::ellipse(mat, center, axes, 0, 0, 360,
                colorToScalar(ann.color), ann.thickness, cv::LINE_AA);
}

static void drawArrow(cv::Mat& mat, const Annotation& ann)
{
    cv::Point pt1(ann.startPoint.x, ann.startPoint.y);
    cv::Point pt2(ann.endPoint.x, ann.endPoint.y);
    cv::arrowedLine(mat, pt1, pt2, colorToScalar(ann.color),
                    ann.thickness, cv::LINE_AA, 0, 0.1);
}

static void drawLine(cv::Mat& mat, const Annotation& ann)
{
    cv::Point pt1(ann.startPoint.x, ann.startPoint.y);
    cv::Point pt2(ann.endPoint.x, ann.endPoint.y);
    cv::line(mat, pt1, pt2, colorToScalar(ann.color), ann.thickness, cv::LINE_AA);
}

static void drawFreehandLine(cv::Mat& mat, const Annotation& ann)
{
    if (ann.points.size() < 2) {
        return;
    }

    std::vector<cv::Point> pts;
    pts.reserve(ann.points.size());
    for (const auto& p : ann.points) {
        pts.emplace_back(static_cast<int>(std::round(p.x)),
                         static_cast<int>(std::round(p.y)));
    }

    cv::polylines(mat, pts, false, colorToScalar(ann.color),
                  ann.thickness, cv::LINE_AA);
}

static void drawText(cv::Mat& mat, const Annotation& ann)
{
    double fontScale = ann.fontSize / 16.0;
    cv::Point org(ann.rect.x, ann.rect.y + ann.fontSize);
    cv::putText(mat, ann.text, org, cv::FONT_HERSHEY_SIMPLEX,
                fontScale, colorToScalar(ann.color), ann.thickness, cv::LINE_AA);
}

static void drawMosaic(cv::Mat& mat, const Annotation& ann)
{
    int blockSize = std::max(1, ann.mosaicBlockSize);

    // Clamp to image bounds
    int x1 = std::max(0, ann.rect.x);
    int y1 = std::max(0, ann.rect.y);
    int x2 = std::min(mat.cols, ann.rect.x + ann.rect.width);
    int y2 = std::min(mat.rows, ann.rect.y + ann.rect.height);

    if (x2 <= x1 || y2 <= y1) {
        return;
    }

    cv::Rect roi(x1, y1, x2 - x1, y2 - y1);
    cv::Mat region = mat(roi);

    // Downscale then upscale to create pixelation effect
    int smallW = std::max(1, region.cols / blockSize);
    int smallH = std::max(1, region.rows / blockSize);

    cv::Mat small;
    cv::resize(region, small, cv::Size(smallW, smallH), 0, 0, cv::INTER_LINEAR);
    cv::resize(small, region, region.size(), 0, 0, cv::INTER_NEAREST);
}

bool ImageProcessUtils::drawAnnotation(ImageData& image, const Annotation& annotation)
{
    if (!image.isValid()) {
        return false;
    }

    cv::Mat mat = imageDataToMat(image);

    switch (annotation.type) {
        case AnnotationType::Rectangle:     drawRectangle(mat, annotation); break;
        case AnnotationType::FilledRect:    drawFilledRect(mat, annotation); break;
        case AnnotationType::Ellipse:       drawEllipse(mat, annotation); break;
        case AnnotationType::Arrow:         drawArrow(mat, annotation); break;
        case AnnotationType::Line:          drawLine(mat, annotation); break;
        case AnnotationType::FreehandLine:  drawFreehandLine(mat, annotation); break;
        case AnnotationType::Text:          drawText(mat, annotation); break;
        case AnnotationType::Mosaic:        drawMosaic(mat, annotation); break;
        default:
            return false;
    }

    return true;
}

int ImageProcessUtils::drawAnnotations(ImageData& image,
                                        const std::vector<Annotation>& annotations)
{
    if (!image.isValid()) {
        return 0;
    }

    int successCount = 0;
    for (const auto& ann : annotations) {
        if (drawAnnotation(image, ann)) {
            ++successCount;
        }
    }
    return successCount;
}

// ============================================================================
// Encoding
// ============================================================================

EncodeResult ImageProcessUtils::encodePng(const ImageData& image)
{
    EncodeResult result;
    if (!image.isValid()) {
        result.errorMessage = "Invalid image data";
        return result;
    }

    // OpenCV imencode expects BGR(A), convert RGBA → BGRA
    cv::Mat rgbaMat = imageDataToMatConst(image);
    cv::Mat bgraMat;
    cv::cvtColor(rgbaMat, bgraMat, cv::COLOR_RGBA2BGRA);

    std::vector<int> params = {cv::IMWRITE_PNG_COMPRESSION, 6};
    std::vector<uint8_t> buf;
    if (!cv::imencode(".png", bgraMat, buf, params)) {
        result.errorMessage = "PNG encoding failed";
        return result;
    }

    result.success = true;
    result.data = std::move(buf);
    return result;
}

EncodeResult ImageProcessUtils::encodeJpeg(const ImageData& image, int quality)
{
    EncodeResult result;
    if (!image.isValid()) {
        result.errorMessage = "Invalid image data";
        return result;
    }

    cv::Mat rgbaMat = imageDataToMatConst(image);
    cv::Mat bgrMat;
    cv::cvtColor(rgbaMat, bgrMat, cv::COLOR_RGBA2BGR);

    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, quality};
    std::vector<uint8_t> buf;
    if (!cv::imencode(".jpg", bgrMat, buf, params)) {
        result.errorMessage = "JPEG encoding failed";
        return result;
    }

    result.success = true;
    result.data = std::move(buf);
    return result;
}

// ============================================================================
// File I/O
// ============================================================================

bool ImageProcessUtils::saveToFile(const ImageData& image, const std::string& filePath)
{
    if (!image.isValid() || filePath.empty()) {
        return false;
    }

    // Construct path with correct UTF-8 handling on all platforms.
    // On Windows, path(std::string) uses ACP (not UTF-8); path(u8string) always uses UTF-8.
    std::filesystem::path p(std::u8string(filePath.begin(), filePath.end()));
    std::string ext = p.extension().string(); // e.g. ".png", ".jpg"
    if (ext.empty()) {
        return false;
    }

    // Encode to memory buffer via imencode (avoids cv::imwrite ANSI path issue on Windows)
    cv::Mat rgbaMat = imageDataToMatConst(image);
    cv::Mat outMat;
    if (ext == ".jpg" || ext == ".jpeg") {
        cv::cvtColor(rgbaMat, outMat, cv::COLOR_RGBA2BGR);
    } else {
        cv::cvtColor(rgbaMat, outMat, cv::COLOR_RGBA2BGRA);
    }

    std::vector<uint8_t> buf;
    if (!cv::imencode(ext, outMat, buf)) {
        return false;
    }

    // Write using std::filesystem::path which handles UTF-8 correctly on all platforms
    std::ofstream ofs(p, std::ios::binary);
    if (!ofs) {
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(buf.data()),
              static_cast<std::streamsize>(buf.size()));
    return ofs.good();
}

// ============================================================================
// Base64
// ============================================================================

std::string ImageProcessUtils::toBase64Png(const ImageData& image)
{
    auto encoded = encodePng(image);
    if (!encoded.isValid()) {
        return {};
    }

    auto b64Result = ucf::utilities::Base64Utils::encode(encoded.data);
    if (!b64Result.isSuccess()) {
        return {};
    }

    return std::move(b64Result.data);
}

// ============================================================================
// Resize
// ============================================================================

ImageData ImageProcessUtils::resize(const ImageData& source, int newWidth, int newHeight)
{
    if (!source.isValid() || newWidth <= 0 || newHeight <= 0) {
        return {};
    }

    cv::Mat srcMat = imageDataToMatConst(source);
    cv::Mat dstMat;
    cv::resize(srcMat, dstMat, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);

    return matToImageData(dstMat);
}

} // namespace ucf::utilities::imageprocess
