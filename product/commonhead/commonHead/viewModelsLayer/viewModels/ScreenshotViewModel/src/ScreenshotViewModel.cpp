#include "ScreenshotViewModel.h"

#include <ucf/Utilities/ScreenCaptureUtils/ScreenCaptureUtils.h>
#include <ucf/Utilities/ImageProcessUtils/ImageProcessUtils.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace commonHead::viewModels {

using namespace ucf::utilities::screencapture;
using namespace ucf::utilities::imageprocess;

// ============================================================================
// Factory
// ============================================================================

std::shared_ptr<IScreenshotViewModel> IScreenshotViewModel::createInstance(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<ScreenshotViewModel>(commonHeadFramework);
}

// ============================================================================
// Construction / Init
// ============================================================================

ScreenshotViewModel::ScreenshotViewModel(commonHead::ICommonHeadFrameworkWptr framework)
    : IScreenshotViewModel(framework)
{
}

std::string ScreenshotViewModel::getViewModelName() const
{
    return "ScreenshotViewModel";
}

void ScreenshotViewModel::init()
{
    // Load default settings — could be extended to read from a persistent config
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
    return ScreenCaptureUtils::hasScreenCapturePermission();
}

void ScreenshotViewModel::requestPermission()
{
    ScreenCaptureUtils::requestScreenCapturePermission();
}

// ============================================================================
// Display / Window Enumeration
// ============================================================================

std::vector<model::DisplayInfoVM> ScreenshotViewModel::getDisplayList() const
{
    auto displays = ScreenCaptureUtils::getDisplayList();
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

std::vector<model::WindowInfoVM> ScreenshotViewModel::getWindowList() const
{
    auto windows = ScreenCaptureUtils::getWindowList();
    std::vector<model::WindowInfoVM> result;
    result.reserve(windows.size());
    for (const auto& w : windows) {
        model::WindowInfoVM vm;
        vm.windowId = w.windowId;
        vm.name = w.name;
        vm.ownerName = w.ownerName;
        result.push_back(std::move(vm));
    }
    return result;
}

// ============================================================================
// Capture
// ============================================================================

void ScreenshotViewModel::onCaptureCompleted(ImageData image, int scaleFactor)
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
        base64 = ImageProcessUtils::toBase64Png(m_capturedImage);
    }

    setState(model::ScreenshotState::Captured);
    fireNotification(&IScreenshotViewModelCallback::onScreenCaptured, base64, width, height);
}

void ScreenshotViewModel::captureFullScreen()
{
    auto captured = ScreenCaptureUtils::captureAllDisplays();
    int scale = captured.scaleFactor;

    auto rgbaImage = ImageProcessUtils::bgraToRgba(
        captured.pixels, captured.width, captured.height, captured.bytesPerRow);

    onCaptureCompleted(std::move(rgbaImage), scale);
}

void ScreenshotViewModel::captureDisplay(int displayIndex)
{
    auto captured = ScreenCaptureUtils::captureDisplay(displayIndex);
    int scale = captured.scaleFactor;

    auto rgbaImage = ImageProcessUtils::bgraToRgba(
        captured.pixels, captured.width, captured.height, captured.bytesPerRow);

    onCaptureCompleted(std::move(rgbaImage), scale);
}

void ScreenshotViewModel::captureWindow(int64_t windowId)
{
    auto captured = ScreenCaptureUtils::captureWindow(windowId);
    int scale = captured.scaleFactor;

    auto rgbaImage = ImageProcessUtils::bgraToRgba(
        captured.pixels, captured.width, captured.height, captured.bytesPerRow);

    onCaptureCompleted(std::move(rgbaImage), scale);
}

// ============================================================================
// Region Selection & Save
// ============================================================================

void ScreenshotViewModel::selectRegionAndSave(int x, int y, int w, int h, double scaleFactor)
{
    setState(model::ScreenshotState::Saving);

    ImageData cropped;
    double effectiveScale;
    {
        std::lock_guard lock(m_mutex);

        effectiveScale = scaleFactor;

        // Scale logical coordinates to physical pixels
        Rect region;
        region.x = static_cast<int>(std::round(x * effectiveScale));
        region.y = static_cast<int>(std::round(y * effectiveScale));
        region.width = static_cast<int>(std::round(w * effectiveScale));
        region.height = static_cast<int>(std::round(h * effectiveScale));

        cropped = ImageProcessUtils::cropRegion(m_capturedImage, region);
    }

    if (!cropped.isValid()) {
        fireNotification(&IScreenshotViewModelCallback::onError,
                         std::string("Region crop failed"));
        setState(model::ScreenshotState::Captured);
        return;
    }

    // Scale annotation coordinates from logical to physical pixels
    {
        std::lock_guard lock(m_mutex);
        for (auto& ann : m_annotations) {
            ann.x = static_cast<int>(std::round(ann.x * effectiveScale));
            ann.y = static_cast<int>(std::round(ann.y * effectiveScale));
            ann.w = static_cast<int>(std::round(ann.w * effectiveScale));
            ann.h = static_cast<int>(std::round(ann.h * effectiveScale));
            ann.startX = static_cast<int>(std::round(ann.startX * effectiveScale));
            ann.startY = static_cast<int>(std::round(ann.startY * effectiveScale));
            ann.endX = static_cast<int>(std::round(ann.endX * effectiveScale));
            ann.endY = static_cast<int>(std::round(ann.endY * effectiveScale));
            ann.thickness = std::max(1, static_cast<int>(std::round(ann.thickness * effectiveScale)));
            ann.fontSize = std::max(8, static_cast<int>(std::round(ann.fontSize * effectiveScale)));
            for (auto& [px, py] : ann.points) {
                px *= effectiveScale;
                py *= effectiveScale;
            }
        }
    }

    // Draw annotations onto cropped image
    cropped = renderAnnotationsOnImage(cropped);

    // Save to file
    std::string filePath;
    {
        std::lock_guard lock(m_mutex);
        std::string dir = m_settings.outputDirectory;
        if (dir.empty()) {
            // Default to user's Desktop
            const char* home = std::getenv("HOME");
            if (home) {
                dir = std::string(home) + "/Desktop";
            } else {
                dir = ".";
            }
        }

        std::filesystem::create_directories(dir);
        filePath = dir + "/" + generateFilename();
    }

    bool saved = ImageProcessUtils::saveToFile(cropped, filePath);
    if (saved) {
        setState(model::ScreenshotState::Idle);
        fireNotification(&IScreenshotViewModelCallback::onScreenshotSaved, filePath);
    } else {
        fireNotification(&IScreenshotViewModelCallback::onError,
                         std::string("Failed to save screenshot to: ") + filePath);
        setState(model::ScreenshotState::Captured);
    }
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

std::string ScreenshotViewModel::saveScreenshot()
{
    ImageData imageToSave;
    std::string filePath;
    {
        std::lock_guard lock(m_mutex);
        if (!m_capturedImage.isValid()) return {};

        fprintf(stderr, "[ScreenshotVM] saveScreenshot: image %dx%d pixels=%zu\n",
                m_capturedImage.width, m_capturedImage.height, m_capturedImage.pixels.size());

        imageToSave = renderAnnotationsOnImage(m_capturedImage);

        std::string dir = m_settings.outputDirectory;
        if (dir.empty()) {
            const char* home = std::getenv("HOME");
            dir = home ? std::string(home) + "/Desktop" : ".";
        }
        std::filesystem::create_directories(dir);
        filePath = dir + "/" + generateFilename();
    }

    if (ImageProcessUtils::saveToFile(imageToSave, filePath)) {
        setState(model::ScreenshotState::Idle);
        fireNotification(&IScreenshotViewModelCallback::onScreenshotSaved, filePath);
        return filePath;
    }

    fireNotification(&IScreenshotViewModelCallback::onError,
                     std::string("Failed to save screenshot"));
    return {};
}

std::string ScreenshotViewModel::getBase64Png() const
{
    std::lock_guard lock(m_mutex);
    if (!m_capturedImage.isValid()) return {};

    auto rendered = renderAnnotationsOnImage(m_capturedImage);
    return ImageProcessUtils::toBase64Png(rendered);
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
    fireNotification(&IScreenshotViewModelCallback::onSettingsChanged, settings);
}

// ============================================================================
// Internal Helpers
// ============================================================================

std::string ScreenshotViewModel::generateFilename() const
{
    // Format: Screenshot_YYYYMMDD_HHmmss.ext
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::ostringstream oss;
    oss << "Screenshot_"
        << std::put_time(&tm, "%Y%m%d_%H%M%S")
        << "." << m_settings.imageFormat;
    return oss.str();
}

ImageData ScreenshotViewModel::renderAnnotationsOnImage(const ImageData& source) const
{
    if (m_annotations.empty()) {
        return source;
    }

    // Convert AnnotationData → Utilities Annotation
    std::vector<Annotation> utilsAnnotations;
    utilsAnnotations.reserve(m_annotations.size());
    for (const auto& ann : m_annotations) {
        utilsAnnotations.push_back(toUtilsAnnotation(ann));
    }

    // Make a copy and draw annotations
    ImageData result = source;
    ImageProcessUtils::drawAnnotations(result, utilsAnnotations);
    return result;
}

Annotation ScreenshotViewModel::toUtilsAnnotation(const model::AnnotationData& ann)
{
    Annotation a;

    // Map type string to enum
    if (ann.type == "rectangle")       a.type = AnnotationType::Rectangle;
    else if (ann.type == "ellipse")    a.type = AnnotationType::Ellipse;
    else if (ann.type == "arrow")      a.type = AnnotationType::Arrow;
    else if (ann.type == "line")       a.type = AnnotationType::Line;
    else if (ann.type == "freehand")   a.type = AnnotationType::FreehandLine;
    else if (ann.type == "text")       a.type = AnnotationType::Text;
    else if (ann.type == "mosaic")     a.type = AnnotationType::Mosaic;
    else if (ann.type == "filledrect") a.type = AnnotationType::FilledRect;
    else                               a.type = AnnotationType::Rectangle;

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

} // namespace commonHead::viewModels
