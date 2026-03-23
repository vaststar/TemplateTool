#pragma once

#include <QObject>
#include <QVariantList>
#include <QtQml>
#include <memory>
#include "UIViewBase/include/UIViewController.h"
#include "ViewModelSingalEmitter/ScreenshotViewModelEmitter.h"

namespace commonHead::viewModels {
    class IScreenshotViewModel;
}

/**
 * @brief Screenshot Controller - thin QML-facing bridge to IScreenshotViewModel
 *
 * Delegates capture, annotation, save/export operations to the ViewModel.
 * Receives ViewModel callbacks via ScreenshotViewModelEmitter and
 * translates them to Q_PROPERTY / Qt signal updates for QML.
 *
 * File-browser helpers (openFile, revealInFinder, etc.) remain in the
 * UI layer because they use platform-specific Qt APIs (QProcess).
 */
class ScreenshotController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    // Screenshot properties
    Q_PROPERTY(bool hasScreenshot READ hasScreenshot NOTIFY screenshotChanged)
    Q_PROPERTY(QString screenshotBase64 READ screenshotBase64 NOTIFY screenshotChanged)
    Q_PROPERTY(int screenshotWidth READ screenshotWidth NOTIFY screenshotChanged)
    Q_PROPERTY(int screenshotHeight READ screenshotHeight NOTIFY screenshotChanged)
    Q_PROPERTY(int screenshotState READ screenshotState NOTIFY stateChanged)

    // Settings
    Q_PROPERTY(QString outputDirectory READ outputDirectory WRITE setOutputDirectory NOTIFY settingsChanged)
    Q_PROPERTY(QString screenshotsFolderUrl READ screenshotsFolderUrl NOTIFY settingsChanged)
    Q_PROPERTY(int delaySeconds READ delaySeconds WRITE setDelaySeconds NOTIFY settingsChanged)
    Q_PROPERTY(QString imageFormat READ imageFormat WRITE setImageFormat NOTIFY settingsChanged)
    Q_PROPERTY(bool includeTimestamp READ includeTimestamp WRITE setIncludeTimestamp NOTIFY settingsChanged)

public:
    explicit ScreenshotController(QObject* parent = nullptr);
    ~ScreenshotController() override;

    // Property getters
    bool hasScreenshot() const;
    QString screenshotBase64() const;
    int screenshotWidth() const;
    int screenshotHeight() const;
    int screenshotState() const;
    QString outputDirectory() const;
    QString screenshotsFolderUrl() const;
    int delaySeconds() const;
    QString imageFormat() const;
    bool includeTimestamp() const;

    // Property setters
    void setOutputDirectory(const QString& path);
    void setDelaySeconds(int seconds);
    void setImageFormat(const QString& format);
    void setIncludeTimestamp(bool include);

    // === Screenshot Methods (delegate to ViewModel) ===
    Q_INVOKABLE void captureFullScreen();
    Q_INVOKABLE void captureWindow(qint64 windowId);
    Q_INVOKABLE QVariantList getWindowList();
    Q_INVOKABLE QString getWindowThumbnailBase64(qint64 windowId);

    // === Annotation Methods (delegate to ViewModel) ===
    Q_INVOKABLE void addRectangle(int x, int y, int w, int h, const QString& color, int thickness);
    Q_INVOKABLE void addEllipse(int x, int y, int w, int h, const QString& color, int thickness);
    Q_INVOKABLE void addArrow(int x1, int y1, int x2, int y2, const QString& color, int thickness);
    Q_INVOKABLE void addText(int x, int y, const QString& text, const QString& color, int fontSize);
    Q_INVOKABLE void addFreehand(const QVariantList& points, const QString& color, int thickness);
    Q_INVOKABLE void addMosaic(int x, int y, int w, int h, int blockSize = 10);
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void clearAnnotations();

    // === Save/Export (delegate to ViewModel) ===
    Q_INVOKABLE void saveScreenshot();
    Q_INVOKABLE void discardScreenshot();
    Q_INVOKABLE QString getDefaultSavePath();

    // === Overlay (delegate to ViewModel) ===
    Q_INVOKABLE QVariantMap grabScreenForOverlay();
    Q_INVOKABLE QVariantMap saveRegionScreenshot(int x, int y, int w, int h,
                                                  int paintedWidth, int paintedHeight,
                                                  const QVariantList& annotations);

    // === File Browser Methods (UI-layer only) ===
    Q_INVOKABLE void openScreenshotsFolder();
    Q_INVOKABLE void openFile(const QString& filePath);
    Q_INVOKABLE void copyFileToClipboard(const QString& filePath);
    Q_INVOKABLE void revealInFinder(const QString& filePath);
    Q_INVOKABLE void deleteFile(const QString& filePath);

    // === Permission ===
    Q_INVOKABLE bool hasPermission();
    Q_INVOKABLE void requestPermission();

protected:
    void init() override;

signals:
    void screenshotChanged();
    void stateChanged();
    void annotationsChanged();
    void settingsChanged();
    void captureCompleted(const QString& filePath);
    void errorOccurred(const QString& message);

private slots:
    // ── ViewModel emitter slots ──
    void onVMStateChanged(int state);
    void onVMScreenCaptured(const QString& base64Png, int width, int height);
    void onVMAnnotationsChanged();
    void onVMScreenshotSaved(const QString& filePath);
    void onVMSettingsChanged(const commonHead::viewModels::model::ScreenshotSettings& settings);
    void onVMError(const QString& message);

private:
    // ViewModel
    std::shared_ptr<commonHead::viewModels::IScreenshotViewModel> m_viewModel;
    std::shared_ptr<UIVMSignalEmitter::ScreenshotViewModelEmitter> m_viewModelEmitter;

    // Shadow state for QML property bindings
    bool m_hasScreenshot = false;
    QString m_screenshotBase64;
    int m_screenshotWidth = 0;
    int m_screenshotHeight = 0;
    int m_state = 0; // ScreenshotState::Idle

    // Settings shadow (for QML bindings)
    QString m_outputDirectory;
    int m_delaySeconds = 0;
    QString m_imageFormat = "png";
    bool m_includeTimestamp = false;
};
