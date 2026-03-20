#pragma once

#include <QObject>
#include <QRect>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

/**
 * @brief Recording Controller - handles screen recording and GIF conversion
 *
 * Provides functionality for:
 * - Full screen recording (via FFmpeg)
 * - Region recording
 * - Pause/resume recording
 * - GIF conversion
 */
class RecordingController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    // Recording properties
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingStateChanged)
    Q_PROPERTY(int recordingDuration READ recordingDuration NOTIFY recordingDurationChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY recordingStateChanged)

    // Settings
    Q_PROPERTY(QString outputDirectory READ outputDirectory WRITE setOutputDirectory NOTIFY settingsChanged)
    Q_PROPERTY(QString videoFormat READ videoFormat WRITE setVideoFormat NOTIFY settingsChanged)
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY settingsChanged)

public:
    explicit RecordingController(QObject* parent = nullptr);
    ~RecordingController() override;

    // Property getters
    bool isRecording() const;
    int recordingDuration() const;
    bool isPaused() const;
    QString outputDirectory() const;
    QString videoFormat() const;
    int fps() const;

    // Property setters
    void setOutputDirectory(const QString& path);
    void setVideoFormat(const QString& format);
    void setFps(int fps);

    // === Recording Methods ===
    Q_INVOKABLE void startRecording(const QString& mode = "fullscreen");
    Q_INVOKABLE void startRegionRecording(int x, int y, int width, int height);
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void pauseRecording();
    Q_INVOKABLE void resumeRecording();
    Q_INVOKABLE void convertToGif(const QString& inputPath, const QString& outputPath = QString());

    // === Utility ===
    Q_INVOKABLE QString getFFmpegPath();
    Q_INVOKABLE bool isFFmpegAvailable();

    // === File Browser Methods ===
    Q_INVOKABLE void openRecordingsFolder();
    Q_INVOKABLE void openFile(const QString& filePath);
    Q_INVOKABLE QString getDefaultSavePath();

protected:
    void init() override;

signals:
    void recordingStateChanged();
    void recordingDurationChanged();
    void settingsChanged();
    void recordingCompleted(const QString& filePath);
    void errorOccurred(const QString& message);

private slots:
    void onRecordingDurationTick();

private:
    void loadSettings();
    void saveSettings();
    QString findFFmpegPath();

    bool m_isRecording = false;
    bool m_isPaused = false;
    int m_recordingDuration = 0;

    QString m_outputDirectory;
    QString m_videoFormat = "mp4";
    int m_fps = 30;

    QString m_ffmpegPath;
    QTimer* m_recordingTimer = nullptr;
};
