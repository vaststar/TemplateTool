#pragma once

#include <memory>
#include <string>
#include <vector>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/ScreenshotViewModel/IScreenshotModel.h>

namespace commonHead {
class ICommonHeadFramework;
using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
} // namespace commonHead

namespace commonHead::viewModels {

// ============================================================================
// Callback interface — UI layer implements this to receive notifications
// ============================================================================

class COMMONHEAD_EXPORT IScreenshotViewModelCallback {
public:
    IScreenshotViewModelCallback() = default;
    IScreenshotViewModelCallback(const IScreenshotViewModelCallback&) = delete;
    IScreenshotViewModelCallback(IScreenshotViewModelCallback&&) = delete;
    IScreenshotViewModelCallback& operator=(const IScreenshotViewModelCallback&) = delete;
    IScreenshotViewModelCallback& operator=(IScreenshotViewModelCallback&&) = delete;
    virtual ~IScreenshotViewModelCallback() = default;

    /// Screenshot state machine transition
    virtual void onStateChanged(model::ScreenshotState state) = 0;

    /// Full screen has been captured; base64 PNG provided for overlay display
    virtual void onScreenCaptured(const std::string& base64Png, int width, int height) = 0;

    /// Annotation list changed (add/remove/undo/redo/clear)
    virtual void onAnnotationsChanged() = 0;

    /// Screenshot saved to disk
    virtual void onScreenshotSaved(const std::string& filePath) = 0;

    /// Settings updated
    virtual void onSettingsChanged(const model::ScreenshotSettings& settings) = 0;

    /// Error occurred
    virtual void onError(const std::string& message) = 0;
};

// ============================================================================
// IScreenshotViewModel — pure interface
// ============================================================================

class COMMONHEAD_EXPORT IScreenshotViewModel
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<IScreenshotViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    ~IScreenshotViewModel() override = default;

    // === Permission ===
    [[nodiscard]] virtual bool hasPermission() const = 0;
    virtual void requestPermission() = 0;

    // === Capture ===
    virtual void captureFullScreen() = 0;
    virtual void captureDisplay(int displayIndex) = 0;
    virtual void captureWindow(int64_t windowId) = 0;

    // === Display / Window Enumeration ===
    [[nodiscard]] virtual std::vector<model::DisplayInfoVM> getDisplayList() const = 0;
    [[nodiscard]] virtual std::vector<model::WindowInfoVM> getWindowList() const = 0;

    // === Region Selection & Save ===
    virtual void selectRegionAndSave(int x, int y, int w, int h, int scaleFactor) = 0;
    virtual void discardCapture() = 0;

    // === Annotation Editing ===
    virtual void addAnnotation(const model::AnnotationData& annotation) = 0;
    virtual void updateAnnotation(int id, const model::AnnotationData& annotation) = 0;
    virtual void removeAnnotation(int id) = 0;
    [[nodiscard]] virtual std::vector<model::AnnotationData> getAnnotations() const = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual void clearAnnotations() = 0;
    [[nodiscard]] virtual bool canUndo() const = 0;
    [[nodiscard]] virtual bool canRedo() const = 0;

    // === Export ===
    virtual std::string saveScreenshot() = 0;
    [[nodiscard]] virtual std::string getBase64Png() const = 0;

    // === Settings ===
    [[nodiscard]] virtual model::ScreenshotSettings getSettings() const = 0;
    virtual void updateSettings(const model::ScreenshotSettings& settings) = 0;

    // === State ===
    [[nodiscard]] virtual model::ScreenshotState getState() const = 0;

    // === Factory ===
    static std::shared_ptr<IScreenshotViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
