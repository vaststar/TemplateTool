#pragma once

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/ScreenshotViewModel/IScreenshotViewModel.h>

#include <ucf/Utilities/ImageProcessUtils/ImageProcessUtils.h>

#include <mutex>

namespace commonHead::viewModels {

class ScreenshotViewModel
    : public virtual IScreenshotViewModel
    , public virtual commonHead::utilities::VMNotificationHelper<IScreenshotViewModelCallback>
{
public:
    explicit ScreenshotViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~ScreenshotViewModel() override = default;

    std::string getViewModelName() const override;

    // === Permission ===
    bool hasPermission() const override;
    void requestPermission() override;

    // === Capture ===
    void captureFullScreen() override;
    void captureDisplay(int displayIndex) override;
    void captureWindow(int64_t windowId) override;

    // === Display / Window Enumeration ===
    std::vector<model::DisplayInfoVM> getDisplayList() const override;
    std::vector<model::WindowInfoVM> getWindowList() const override;

    // === Region Selection & Save ===
    void selectRegionAndSave(int x, int y, int w, int h, int scaleFactor) override;
    void discardCapture() override;

    // === Annotation Editing ===
    void addAnnotation(const model::AnnotationData& annotation) override;
    void updateAnnotation(int id, const model::AnnotationData& annotation) override;
    void removeAnnotation(int id) override;
    std::vector<model::AnnotationData> getAnnotations() const override;
    void undo() override;
    void redo() override;
    void clearAnnotations() override;
    bool canUndo() const override;
    bool canRedo() const override;

    // === Export ===
    std::string saveScreenshot() override;
    std::string getBase64Png() const override;

    // === Settings ===
    model::ScreenshotSettings getSettings() const override;
    void updateSettings(const model::ScreenshotSettings& settings) override;

    // === State ===
    model::ScreenshotState getState() const override;

protected:
    void init() override;

private:
    // Internal helpers
    void setState(model::ScreenshotState newState);
    void onCaptureCompleted(ucf::utilities::imageprocess::ImageData image, int scaleFactor);
    ucf::utilities::imageprocess::ImageData renderAnnotationsOnImage(
        const ucf::utilities::imageprocess::ImageData& source) const;
    std::string generateFilename() const;
    int nextAnnotationId();

    // Convert between ViewModel AnnotationData and Utilities Annotation
    static ucf::utilities::imageprocess::Annotation toUtilsAnnotation(const model::AnnotationData& ann);

private:
    mutable std::mutex m_mutex;

    // State
    model::ScreenshotState m_state = model::ScreenshotState::Idle;
    model::ScreenshotSettings m_settings;

    // Captured full-screen image (RGBA)
    ucf::utilities::imageprocess::ImageData m_capturedImage;
    int m_captureScaleFactor = 1;

    // Annotation undo/redo stack
    std::vector<model::AnnotationData> m_annotations;
    std::vector<std::vector<model::AnnotationData>> m_undoStack;
    std::vector<std::vector<model::AnnotationData>> m_redoStack;
    int m_nextAnnotationId = 1;
};

} // namespace commonHead::viewModels
