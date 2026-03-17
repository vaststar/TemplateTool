#pragma once

#include <QObject>
#include <QImage>
#include <QVariantList>
#include <QRect>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

class ScreenshotService;

/**
 * @brief Screen Capture Controller - handles screenshot and screen recording
 *
 * Provides functionality for:
 * - Full screen capture
 * - Region selection capture
 * - Window capture (platform-specific)
 * - Screenshot editing with annotations
 * - Screen recording (via FFmpeg)
 */
class ScreenCaptureController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    // Screenshot properties
    Q_PROPERTY(QImage currentScreenshot READ currentScreenshot NOTIFY screenshotChanged)
    Q_PROPERTY(bool hasScreenshot READ hasScreenshot NOTIFY screenshotChanged)
    Q_PROPERTY(QString screenshotBase64 READ screenshotBase64 NOTIFY screenshotChanged)
    Q_PROPERTY(int screenshotWidth READ screenshotWidth NOTIFY screenshotChanged)
    Q_PROPERTY(int screenshotHeight READ screenshotHeight NOTIFY screenshotChanged)
    Q_PROPERTY(QVariantList annotations READ annotations NOTIFY annotationsChanged)

    // Recording properties
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingStateChanged)
    Q_PROPERTY(int recordingDuration READ recordingDuration NOTIFY recordingDurationChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY recordingStateChanged)

    // Settings
    Q_PROPERTY(QString outputDirectory READ outputDirectory WRITE setOutputDirectory NOTIFY settingsChanged)
    Q_PROPERTY(QString screenshotsFolderUrl READ screenshotsFolderUrl NOTIFY settingsChanged)
    Q_PROPERTY(int delaySeconds READ delaySeconds WRITE setDelaySeconds NOTIFY settingsChanged)
    Q_PROPERTY(QString imageFormat READ imageFormat WRITE setImageFormat NOTIFY settingsChanged)
    Q_PROPERTY(QString videoFormat READ videoFormat WRITE setVideoFormat NOTIFY settingsChanged)
    Q_PROPERTY(bool includeTimestamp READ includeTimestamp WRITE setIncludeTimestamp NOTIFY settingsChanged)
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY settingsChanged)

public:
    explicit ScreenCaptureController(QObject* parent = nullptr);
    ~ScreenCaptureController() override;

    // Property getters
    QImage currentScreenshot() const;
    bool hasScreenshot() const;
    QString screenshotBase64() const;
    int screenshotWidth() const;
    int screenshotHeight() const;
    QVariantList annotations() const;
    bool isRecording() const;
    int recordingDuration() const;
    bool isPaused() const;
    QString outputDirectory() const;
    QString screenshotsFolderUrl() const;  // Returns file:// URL for QML FolderListModel
    int delaySeconds() const;
    QString imageFormat() const;
    QString videoFormat() const;
    bool includeTimestamp() const;
    int fps() const;

    // Property setters
    void setOutputDirectory(const QString& path);
    void setDelaySeconds(int seconds);
    void setImageFormat(const QString& format);
    void setVideoFormat(const QString& format);
    void setIncludeTimestamp(bool include);
    void setFps(int fps);

    // === Screenshot Methods ===
    Q_INVOKABLE void captureFullScreen();
    Q_INVOKABLE void captureRegion(int x, int y, int width, int height);
    Q_INVOKABLE void captureInteractiveRegion();  // Use system's interactive selection
    Q_INVOKABLE void captureWithDelay(int seconds, const QString& mode = "fullscreen");
    Q_INVOKABLE QVariantList getWindowList();
    Q_INVOKABLE void captureWindow(qint64 windowId);
    Q_INVOKABLE void loadFromClipboard();  // Load screenshot from clipboard

    // === Editing Methods ===
    Q_INVOKABLE void addRectangle(int x, int y, int w, int h, const QString& color, int thickness);
    Q_INVOKABLE void addEllipse(int x, int y, int w, int h, const QString& color, int thickness);
    Q_INVOKABLE void addArrow(int x1, int y1, int x2, int y2, const QString& color, int thickness);
    Q_INVOKABLE void addText(int x, int y, const QString& text, const QString& color, int fontSize);
    Q_INVOKABLE void addFreehand(const QVariantList& points, const QString& color, int thickness);
    Q_INVOKABLE void addMosaic(int x, int y, int w, int h, int blockSize = 10);
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void clearAnnotations();

    // === Save/Export Methods ===
    Q_INVOKABLE bool saveScreenshot(const QString& filePath = QString());
    Q_INVOKABLE void copyToClipboard();
    Q_INVOKABLE QString generateFilename(const QString& extension = "png");
    Q_INVOKABLE QString getDefaultSavePath();

    // === Recording Methods ===
    Q_INVOKABLE void startRecording(const QString& mode = "fullscreen");
    Q_INVOKABLE void startRegionRecording(int x, int y, int width, int height);
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void pauseRecording();
    Q_INVOKABLE void resumeRecording();
    Q_INVOKABLE void convertToGif(const QString& inputPath, const QString& outputPath = QString());

    // === Utility ===
    Q_INVOKABLE void discardScreenshot();
    Q_INVOKABLE QString getFFmpegPath();
    Q_INVOKABLE bool isFFmpegAvailable();

    // === File Browser Methods ===
    Q_INVOKABLE void openScreenshotsFolder();
    Q_INVOKABLE void openFile(const QString& filePath);
    Q_INVOKABLE void copyFileToClipboard(const QString& filePath);
    Q_INVOKABLE void revealInFinder(const QString& filePath);
    Q_INVOKABLE void deleteFile(const QString& filePath);

    // === Overlay Screenshot Methods ===
    Q_INVOKABLE QVariantMap grabScreenForOverlay();  // Returns {success, base64, width, height}
    Q_INVOKABLE QVariantMap saveRegionScreenshot(int x, int y, int w, int h, int windowWidth, int windowHeight, const QVariantList& annotations);  // Returns {success, filePath, error}

protected:
    void init() override;

signals:
    void screenshotChanged();
    void annotationsChanged();
    void recordingStateChanged();
    void recordingDurationChanged();
    void settingsChanged();
    void captureCompleted(const QString& filePath);
    void recordingCompleted(const QString& filePath);
    void errorOccurred(const QString& message);
    void delayCountdown(int secondsRemaining);

private slots:
    void onDelayTimerTick();
    void onRecordingDurationTick();

private:
    void loadSettings();
    void saveSettings();
    QImage renderAnnotations(const QImage& base);
    QString findFFmpegPath();

    std::unique_ptr<ScreenshotService> m_screenshotService;

    QImage m_currentScreenshot;
    QVariantList m_annotations;
    QVariantList m_undoStack;
    QVariantList m_redoStack;

    bool m_isRecording = false;
    bool m_isPaused = false;
    int m_recordingDuration = 0;

    QString m_outputDirectory;
    int m_delaySeconds = 0;
    QString m_imageFormat = "png";
    QString m_videoFormat = "mp4";
    bool m_includeTimestamp = true;
    int m_fps = 30;

    QString m_ffmpegPath;
    QTimer* m_delayTimer = nullptr;
    QTimer* m_recordingTimer = nullptr;
    int m_delayCountdown = 0;
    QString m_pendingCaptureMode;
    QRect m_pendingRegion;
};
