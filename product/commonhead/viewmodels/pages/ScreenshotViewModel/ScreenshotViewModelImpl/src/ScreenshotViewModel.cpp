#include "ScreenshotViewModel.h"
#include "LoggerDefine.h"

#include <ucf/utilities/ScreenCaptureUtils/ScreenCaptureUtils.h>
#include <ucf/utilities/ImageProcessUtils/ImageProcessUtils.h>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ServiceLocator/IServiceLocator.h>
#include <commonhead/viewmodels/ViewModelUtils/TimeDisplayUtils.h>
#include <commonhead/viewmodels/ScreenshotViewModel/ScreenshotViewModelCreator.h>
#include <ucf/services/FeatureSettingsService/IFeatureSettingsService.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <limits>
#include <stdexcept>
#include <utility>
#include <cmath>
#include <filesystem>
#include <sstream>

namespace commonHead::viewModels {

namespace {

int scaleToPixel(double value, double scaleFactor)
{
    const double scaled = std::round(value * scaleFactor);
    if (!std::isfinite(scaled)
        || scaled < std::numeric_limits<int>::min()
        || scaled > std::numeric_limits<int>::max()) {
        throw std::invalid_argument("Screenshot coordinates are outside the supported range");
    }
    return static_cast<int>(scaled);
}

template <typename Notify>
void notifySaveCallbacksSafely(Notify&& notify) noexcept
{
    try {
        std::forward<Notify>(notify)();
    } catch (...) {
        // Observer failures must not change the actual export result.
        try {
            SCREENSHOT_VIEW_MODEL_LOG_WARN("Screenshot callback threw; export result preserved");
        } catch (...) {
        }
    }
}

} // namespace

// ============================================================================
// Factory
// ============================================================================

namespace impl {

std::shared_ptr<IScreenshotViewModel> createScreenshotViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<ScreenshotViewModel>(commonHeadFramework);
}

} // namespace impl

// ============================================================================
// Construction / Init
// ============================================================================

ScreenshotViewModel::ScreenshotViewModel(commonHead::ICommonHeadFrameworkWptr framework)
    : IScreenshotViewModel(framework)
{
    SCREENSHOT_VIEW_MODEL_LOG_DEBUG("ScreenshotViewModel constructed, address: " << this);
}

ScreenshotViewModel::~ScreenshotViewModel()
{
    SCREENSHOT_VIEW_MODEL_LOG_DEBUG("ScreenshotViewModel destroying, address: " << this);
}

std::string ScreenshotViewModel::getViewModelName() const
{
    return "ScreenshotViewModel";
}

void ScreenshotViewModel::init()
{
    // Load settings from FeatureSettingsService if available, otherwise use defaults
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto featureSettingsService = serviceLocator->getFeatureSettingsService().lock())
            {
                auto serviceSettings = featureSettingsService->getScreenshotSettings();
                m_settings.outputDirectory = serviceSettings.outputDirectory;
                m_settings.imageFormat = serviceSettings.imageFormat;
                m_settings.jpegQuality = serviceSettings.jpegQuality;
                m_settings.captureDelay = serviceSettings.captureDelay;
                m_settings.addTimestamp = serviceSettings.addTimestamp;
                return;
            }
        }
    }
    // Fallback defaults
    m_settings.outputDirectory = "";
    m_settings.imageFormat = "png";
    m_settings.jpegQuality = 90;
    m_settings.captureDelay = 0;
    m_settings.addTimestamp = false;
}

// ============================================================================
// State Management
// ============================================================================

model::ScreenshotState ScreenshotViewModel::getState() const
{
    std::lock_guard lock(m_mutex);
    return m_state;
}

void ScreenshotViewModel::setState(model::ScreenshotState newState)
{
    {
        std::lock_guard lock(m_mutex);
        if (m_state == newState) return;
        m_state = newState;
    }
    fireNotification(&IScreenshotViewModelCallback::onStateChanged, newState);
}

// ============================================================================
// Permission
// ============================================================================

bool ScreenshotViewModel::hasPermission() const
{
    return ucf::utilities::screencapture::ScreenCaptureUtils::hasScreenCapturePermission();
}

void ScreenshotViewModel::requestPermission()
{
    ucf::utilities::screencapture::ScreenCaptureUtils::requestScreenCapturePermission();
}

// ============================================================================
// Display / Window Enumeration
// ============================================================================

std::vector<model::DisplayInfoVM> ScreenshotViewModel::getDisplayList() const
{
    auto displays = ucf::utilities::screencapture::ScreenCaptureUtils::getDisplayList();
    std::vector<model::DisplayInfoVM> result;
    result.reserve(displays.size());
    for (const auto& d : displays) {
        model::DisplayInfoVM vm;
        vm.displayId = d.displayId;
        vm.name = d.name;
        vm.x = d.x;
        vm.y = d.y;
        vm.width = d.width;
        vm.height = d.height;
        vm.scaleFactor = d.scaleFactor;
        vm.isPrimary = d.isPrimary;
        result.push_back(std::move(vm));
    }
    return result;
}

// ============================================================================
// Capture
// ============================================================================

void ScreenshotViewModel::onCaptureCompleted(ucf::utilities::imageprocess::ImageData image, int scaleFactor)
{
    if (!image.isValid()) {
        fireNotification(&IScreenshotViewModelCallback::onError,
                         std::string("Screen capture failed"));
        return;
    }

    std::string base64;
    int width, height;
    {
        std::lock_guard lock(m_mutex);
        m_capturedImage = std::move(image);
        m_captureScaleFactor = scaleFactor;
        m_annotations.clear();
        m_undoStack.clear();
        m_redoStack.clear();
        m_nextAnnotationId = 1;

        width = m_capturedImage.width;
        height = m_capturedImage.height;
        base64 = ucf::utilities::imageprocess::ImageProcessUtils::toBase64Png(m_capturedImage);
    }

    setState(model::ScreenshotState::Captured);
    fireNotification(&IScreenshotViewModelCallback::onScreenCaptured, base64, width, height);
}

void ScreenshotViewModel::captureFullScreen()
{
    auto captured = ucf::utilities::screencapture::ScreenCaptureUtils::captureAllDisplays();
    int scale = captured.scaleFactor;

    auto rgbaImage = ucf::utilities::imageprocess::ImageProcessUtils::bgraToRgba(
        captured.pixels, captured.width, captured.height, captured.bytesPerRow);

    onCaptureCompleted(std::move(rgbaImage), scale);
}

void ScreenshotViewModel::captureDisplay(int displayIndex)
{
    auto captured = ucf::utilities::screencapture::ScreenCaptureUtils::captureDisplay(displayIndex);
    int scale = captured.scaleFactor;

    auto rgbaImage = ucf::utilities::imageprocess::ImageProcessUtils::bgraToRgba(
        captured.pixels, captured.width, captured.height, captured.bytesPerRow);

    onCaptureCompleted(std::move(rgbaImage), scale);
}

// ============================================================================
// Region Selection & Save
// ============================================================================

model::ScreenshotSaveResult ScreenshotViewModel::selectRegionAndSave(
    int x, int y, int w, int h, double scaleFactor,
    const std::vector<model::AnnotationData>& annotations)
{
    return saveCapturedImage(
        ucf::utilities::imageprocess::Rect{x, y, w, h}, scaleFactor, &annotations);
}

void ScreenshotViewModel::discardCapture()
{
    {
        std::lock_guard lock(m_mutex);
        m_capturedImage = {};
        m_annotations.clear();
        m_undoStack.clear();
        m_redoStack.clear();
    }
    setState(model::ScreenshotState::Idle);
}

// ============================================================================
// Annotation Editing
// ============================================================================

int ScreenshotViewModel::nextAnnotationId()
{
    return m_nextAnnotationId++;
}

void ScreenshotViewModel::addAnnotation(const model::AnnotationData& annotation)
{
    {
        std::lock_guard lock(m_mutex);
        m_undoStack.push_back(m_annotations);
        m_redoStack.clear();

        model::AnnotationData ann = annotation;
        ann.id = nextAnnotationId();
        m_annotations.push_back(std::move(ann));
    }

    if (m_state == model::ScreenshotState::Captured) {
        setState(model::ScreenshotState::Editing);
    }
    fireNotification(&IScreenshotViewModelCallback::onAnnotationsChanged);
}

void ScreenshotViewModel::updateAnnotation(int id, const model::AnnotationData& annotation)
{
    {
        std::lock_guard lock(m_mutex);
        for (auto& ann : m_annotations) {
            if (ann.id == id) {
                m_undoStack.push_back(m_annotations);
                m_redoStack.clear();
                ann = annotation;
                ann.id = id; // preserve id
                break;
            }
        }
    }
    fireNotification(&IScreenshotViewModelCallback::onAnnotationsChanged);
}

void ScreenshotViewModel::removeAnnotation(int id)
{
    {
        std::lock_guard lock(m_mutex);
        m_undoStack.push_back(m_annotations);
        m_redoStack.clear();

        m_annotations.erase(
            std::remove_if(m_annotations.begin(), m_annotations.end(),
                           [id](const model::AnnotationData& a) { return a.id == id; }),
            m_annotations.end());
    }
    fireNotification(&IScreenshotViewModelCallback::onAnnotationsChanged);
}

std::vector<model::AnnotationData> ScreenshotViewModel::getAnnotations() const
{
    std::lock_guard lock(m_mutex);
    return m_annotations;
}

void ScreenshotViewModel::undo()
{
    {
        std::lock_guard lock(m_mutex);
        if (m_undoStack.empty()) return;
        m_redoStack.push_back(m_annotations);
        m_annotations = m_undoStack.back();
        m_undoStack.pop_back();
    }
    fireNotification(&IScreenshotViewModelCallback::onAnnotationsChanged);
}

void ScreenshotViewModel::redo()
{
    {
        std::lock_guard lock(m_mutex);
        if (m_redoStack.empty()) return;
        m_undoStack.push_back(m_annotations);
        m_annotations = m_redoStack.back();
        m_redoStack.pop_back();
    }
    fireNotification(&IScreenshotViewModelCallback::onAnnotationsChanged);
}

void ScreenshotViewModel::clearAnnotations()
{
    {
        std::lock_guard lock(m_mutex);
        if (m_annotations.empty()) return;
        m_undoStack.push_back(m_annotations);
        m_redoStack.clear();
        m_annotations.clear();
    }
    fireNotification(&IScreenshotViewModelCallback::onAnnotationsChanged);
}

bool ScreenshotViewModel::canUndo() const
{
    std::lock_guard lock(m_mutex);
    return !m_undoStack.empty();
}

bool ScreenshotViewModel::canRedo() const
{
    std::lock_guard lock(m_mutex);
    return !m_redoStack.empty();
}

// ============================================================================
// Export
// ============================================================================

model::ScreenshotSaveResult ScreenshotViewModel::saveScreenshot()
{
    return saveCapturedImage(std::nullopt, 1.0, nullptr);
}

model::ScreenshotSaveResult ScreenshotViewModel::saveCapturedImage(
    const std::optional<ucf::utilities::imageprocess::Rect>& logicalRegion,
    double scaleFactor,
    const std::vector<model::AnnotationData>* overlayAnnotations)
{
    using Status = model::ScreenshotSaveStatus;
    model::ScreenshotSaveResult result;
    ucf::utilities::imageprocess::ImageData image;
    model::ScreenshotSettings settings;
    std::vector<model::AnnotationData> annotations;
    bool startedSaving = false;

    try {
        {
            std::lock_guard lock(m_mutex);
            if (m_state == model::ScreenshotState::Saving) {
                return {Status::Busy, {}, "A screenshot is already being saved"};
            }
            if (!m_capturedImage.isValid()) {
                result = {Status::NoCapture, {}, "No screenshot is available"};
            } else {
                // Copy everything before publishing Saving; copying can itself fail.
                image = m_capturedImage;
                settings = m_settings;
                annotations = overlayAnnotations ? *overlayAnnotations : m_annotations;
                m_state = model::ScreenshotState::Saving;
                startedSaving = true;
            }
        }

        if (startedSaving) {
            notifySaveCallbacksSafely([this] {
                fireNotification(&IScreenshotViewModelCallback::onStateChanged,
                                 model::ScreenshotState::Saving);
            });

            result = [&]() -> model::ScreenshotSaveResult {
                if (logicalRegion) {
                    result.status = Status::InvalidInput;
                    if (!std::isfinite(scaleFactor) || scaleFactor <= 0.0
                        || logicalRegion->x < 0 || logicalRegion->y < 0
                        || logicalRegion->width <= 0 || logicalRegion->height <= 0) {
                        return {Status::InvalidInput, {}, "Invalid screenshot region or scale"};
                    }

                    ucf::utilities::imageprocess::Rect region{
                        scaleToPixel(logicalRegion->x, scaleFactor),
                        scaleToPixel(logicalRegion->y, scaleFactor),
                        scaleToPixel(logicalRegion->width, scaleFactor),
                        scaleToPixel(logicalRegion->height, scaleFactor)
                    };
                    // cropRegion adds coordinates and sizes using int arithmetic.
                    if (region.width <= 0 || region.height <= 0
                        || region.x > std::numeric_limits<int>::max() - region.width
                        || region.y > std::numeric_limits<int>::max() - region.height) {
                        return {Status::InvalidInput, {}, "Screenshot region is outside the supported range"};
                    }

                    result.status = Status::CropFailed;
                    image = ucf::utilities::imageprocess::ImageProcessUtils::cropRegion(image, region);
                    if (!image.isValid()) {
                        return {Status::CropFailed, {}, "Region crop failed"};
                    }

                    result.status = Status::InvalidInput;
                    for (auto& ann : annotations) {
                        ann.x = scaleToPixel(ann.x, scaleFactor);
                        ann.y = scaleToPixel(ann.y, scaleFactor);
                        ann.w = scaleToPixel(ann.w, scaleFactor);
                        ann.h = scaleToPixel(ann.h, scaleFactor);
                        ann.startX = scaleToPixel(ann.startX, scaleFactor);
                        ann.startY = scaleToPixel(ann.startY, scaleFactor);
                        ann.endX = scaleToPixel(ann.endX, scaleFactor);
                        ann.endY = scaleToPixel(ann.endY, scaleFactor);
                        ann.thickness = std::max(1, scaleToPixel(ann.thickness, scaleFactor));
                        ann.fontSize = std::max(8, scaleToPixel(ann.fontSize, scaleFactor));
                        for (auto& [px, py] : ann.points) {
                            px *= scaleFactor;
                            py *= scaleFactor;
                            // The rasterizer converts rounded points to int.
                            (void)scaleToPixel(px, 1.0);
                            (void)scaleToPixel(py, 1.0);
                        }
                    }
                }

                result.status = Status::RenderFailed;
                image = renderAnnotationsOnImage(std::move(image), annotations);
                if (settings.addTimestamp) {
                    addTimestampWatermark(image);
                }
                if (!image.isValid()) {
                    return {Status::RenderFailed, {}, "Screenshot rendering failed"};
                }

                result.status = Status::DirectoryCreationFailed;
                std::string directory = settings.outputDirectory;
                if (directory.empty()) {
                    const char* userHome = std::getenv("HOME");
#if defined(_WIN32)
                    if (!userHome) userHome = std::getenv("USERPROFILE");
#endif
                    directory = userHome ? std::string(userHome) + "/Desktop" : ".";
                }
                const std::filesystem::path outputDirectory(
                    std::u8string(directory.begin(), directory.end()));
                const auto outputPath = outputDirectory / generateFilename(settings.imageFormat);
                const auto utf8Path = outputPath.u8string();
                result.filePath.assign(utf8Path.begin(), utf8Path.end());

                std::error_code directoryError;
                std::filesystem::create_directories(outputDirectory, directoryError);
                if (directoryError) {
                    result.errorMessage = "Could not create screenshot folder: " + directoryError.message();
                    return result;
                }

                result.status = Status::WriteFailed;
                if (!ucf::utilities::imageprocess::ImageProcessUtils::saveToFile(image, result.filePath)) {
                    result.errorMessage = "Failed to save screenshot to: " + result.filePath;
                    return result;
                }

                result.status = Status::Success;
                result.errorMessage.clear();
                return result;
            }();
        }
    } catch (const std::invalid_argument& error) {
        result.status = Status::InvalidInput;
        result.errorMessage = error.what();
    } catch (const std::exception& error) {
        result.errorMessage = error.what();
    } catch (...) {
        result.errorMessage = "Unknown screenshot export error";
    }

    // The export result is final before publishing completion notifications.
    if (startedSaving) {
        notifySaveCallbacksSafely([this, &result] {
            setState(result.succeeded() ? model::ScreenshotState::Idle
                                       : model::ScreenshotState::Captured);
        });
    }
    notifySaveCallbacksSafely([this, &result] {
        if (result.succeeded()) {
            fireNotification(&IScreenshotViewModelCallback::onScreenshotSaved, result.filePath);
        } else {
            fireNotification(&IScreenshotViewModelCallback::onError, result.errorMessage);
        }
    });
    return result;
}

std::string ScreenshotViewModel::getBase64Png() const
{
    ucf::utilities::imageprocess::ImageData image;
    std::vector<model::AnnotationData> annotations;
    {
        std::lock_guard lock(m_mutex);
        if (!m_capturedImage.isValid()) return {};
        image = m_capturedImage;
        annotations = m_annotations;
    }
    auto rendered = renderAnnotationsOnImage(std::move(image), annotations);
    return ucf::utilities::imageprocess::ImageProcessUtils::toBase64Png(rendered);
}

// ============================================================================
// Settings
// ============================================================================

model::ScreenshotSettings ScreenshotViewModel::getSettings() const
{
    std::lock_guard lock(m_mutex);
    return m_settings;
}

void ScreenshotViewModel::updateSettings(const model::ScreenshotSettings& settings)
{
    {
        std::lock_guard lock(m_mutex);
        m_settings = settings;
    }
    // Persist to FeatureSettingsService
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto featureSettingsService = serviceLocator->getFeatureSettingsService().lock())
            {
                ucf::service::model::ScreenshotFeatureSettings serviceSettings;
                serviceSettings.outputDirectory = settings.outputDirectory;
                serviceSettings.imageFormat = settings.imageFormat;
                serviceSettings.jpegQuality = settings.jpegQuality;
                serviceSettings.captureDelay = settings.captureDelay;
                serviceSettings.addTimestamp = settings.addTimestamp;
                featureSettingsService->updateScreenshotSettings(serviceSettings);
            }
        }
    }
    fireNotification(&IScreenshotViewModelCallback::onSettingsChanged, settings);
}

// ============================================================================
// Internal Helpers
// ============================================================================

std::string ScreenshotViewModel::generateFilename(const std::string& imageFormat)
{
    // Format: Screenshot_YYYYMMDD_HHmmss.ext
    const auto timestamp =
        commonHead::utilities::TimeDisplayUtils::formatCurrentUserTime({
            .localPattern = "%Y%m%d_%H%M%S",
            .utcFallbackPattern = "%Y%m%d_%H%M%SZ",
            .failureText = "unknown"
        });

    return "Screenshot_" + timestamp + "." + imageFormat;
}

ucf::utilities::imageprocess::ImageData ScreenshotViewModel::renderAnnotationsOnImage(
    ucf::utilities::imageprocess::ImageData source,
    const std::vector<model::AnnotationData>& annotations)
{
    if (annotations.empty()) {
        return source;
    }

    std::vector<ucf::utilities::imageprocess::Annotation> utilsAnnotations;
    utilsAnnotations.reserve(annotations.size());
    for (const auto& ann : annotations) {
        utilsAnnotations.push_back(toUtilsAnnotation(ann));
    }
    ucf::utilities::imageprocess::ImageProcessUtils::drawAnnotations(source, utilsAnnotations);
    return source;
}

ucf::utilities::imageprocess::Annotation ScreenshotViewModel::toUtilsAnnotation(const model::AnnotationData& ann)
{
    ucf::utilities::imageprocess::Annotation a;

    // Map type string to enum
    if (ann.type == "rectangle")       a.type = ucf::utilities::imageprocess::AnnotationType::Rectangle;
    else if (ann.type == "ellipse")    a.type = ucf::utilities::imageprocess::AnnotationType::Ellipse;
    else if (ann.type == "arrow")      a.type = ucf::utilities::imageprocess::AnnotationType::Arrow;
    else if (ann.type == "line")       a.type = ucf::utilities::imageprocess::AnnotationType::Line;
    else if (ann.type == "freehand")   a.type = ucf::utilities::imageprocess::AnnotationType::FreehandLine;
    else if (ann.type == "text")       a.type = ucf::utilities::imageprocess::AnnotationType::Text;
    else if (ann.type == "mosaic")     a.type = ucf::utilities::imageprocess::AnnotationType::Mosaic;
    else if (ann.type == "filledrect") a.type = ucf::utilities::imageprocess::AnnotationType::FilledRect;
    else                               a.type = ucf::utilities::imageprocess::AnnotationType::Rectangle;

    // The rasterizer adds rectangle bounds and the text baseline using int.
    auto validateSum = [](int first, int second) {
        const auto sum = static_cast<std::int64_t>(first) + second;
        if (sum < std::numeric_limits<int>::min() || sum > std::numeric_limits<int>::max()) {
            throw std::invalid_argument("Screenshot annotation is outside the supported range");
        }
    };
    using Type = ucf::utilities::imageprocess::AnnotationType;
    if (a.type == Type::Rectangle || a.type == Type::Ellipse
        || a.type == Type::Mosaic || a.type == Type::FilledRect) {
        if (ann.w < 0 || ann.h < 0) {
            throw std::invalid_argument("Screenshot annotation dimensions must not be negative");
        }
        validateSum(ann.x, ann.w);
        validateSum(ann.y, ann.h);
    } else if (a.type == Type::Text) {
        validateSum(ann.y, ann.fontSize);
    }
    for (const auto& [px, py] : ann.points) {
        (void)scaleToPixel(px, 1.0);
        (void)scaleToPixel(py, 1.0);
    }

    a.color = {ann.r, ann.g, ann.b, ann.a};
    a.thickness = ann.thickness;

    a.rect = {ann.x, ann.y, ann.w, ann.h};
    a.startPoint = {ann.startX, ann.startY};
    a.endPoint = {ann.endX, ann.endY};

    a.points.reserve(ann.points.size());
    for (const auto& [px, py] : ann.points) {
        a.points.push_back({px, py});
    }

    a.text = ann.text;
    a.fontSize = ann.fontSize;
    a.mosaicBlockSize = ann.mosaicBlockSize;

    return a;
}

void ScreenshotViewModel::addTimestampWatermark(ucf::utilities::imageprocess::ImageData& image) const
{
    if (!image.isValid()) return;

    // Generate timestamp string: "YYYY-MM-DD HH:MM:SS"
    const auto timestampText =
        commonHead::utilities::TimeDisplayUtils::formatCurrentUserTime({
            .localPattern = "%Y-%m-%d %H:%M:%S",
            .utcFallbackPattern = "%Y-%m-%d %H:%M:%SZ",
            .failureText = "Time unavailable"
        });

    // Calculate font size based on image dimensions (roughly 2% of the shorter side)
    int shorterSide = std::min(image.width, image.height);
    int fontSize = std::max(12, shorterSide / 50);

    // Position: bottom-right corner with padding
    int padding = fontSize;
    // For FONT_HERSHEY_SIMPLEX, character width ≈ fontScale * 20 = (fontSize/16) * 20 = fontSize * 1.25
    int textWidth = static_cast<int>(timestampText.length() * fontSize * 1.3);
    int textX = std::max(padding, image.width - textWidth - padding);
    int textY = image.height - padding - fontSize;

    // Draw shadow (dark offset) for readability
    ucf::utilities::imageprocess::Annotation shadowAnnotation;
    shadowAnnotation.type = ucf::utilities::imageprocess::AnnotationType::Text;
    shadowAnnotation.rect = {textX + 1, textY + 1, 0, 0};
    shadowAnnotation.text = timestampText;
    shadowAnnotation.fontSize = fontSize;
    shadowAnnotation.color = {0, 0, 0, 180};
    shadowAnnotation.thickness = 1;
    ucf::utilities::imageprocess::ImageProcessUtils::drawAnnotation(image, shadowAnnotation);

    // Draw main text (white)
    ucf::utilities::imageprocess::Annotation textAnnotation;
    textAnnotation.type = ucf::utilities::imageprocess::AnnotationType::Text;
    textAnnotation.rect = {textX, textY, 0, 0};
    textAnnotation.text = timestampText;
    textAnnotation.fontSize = fontSize;
    textAnnotation.color = {255, 255, 255, 230};
    textAnnotation.thickness = 1;
    ucf::utilities::imageprocess::ImageProcessUtils::drawAnnotation(image, textAnnotation);
}

} // namespace commonHead::viewModels
