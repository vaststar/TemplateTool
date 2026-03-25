#pragma once

#include <QObject>
#include <QRect>
#include <QtQml>
#include <memory>
#include "UIViewBase/include/UIViewController.h"
#include "ViewModelSingalEmitter/RecordingViewModelEmitter.h"

namespace commonHead::viewModels {
    class IRecordingViewModel;
}

/**
 * @brief Recording Controller - thin QML-facing bridge to IRecordingViewModel
 *
 * Delegates recording control and settings to the ViewModel.
 * Receives ViewModel callbacks via RecordingViewModelEmitter and
 * translates them to Q_PROPERTY / Qt signal updates for QML.
 *
 * File-browser helpers (openFile, openRecordingsFolder) remain in the
 * UI layer because they use platform-specific Qt APIs (QProcess).
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

    // FFmpeg status (reactive properties for QML binding)
    Q_PROPERTY(bool ffmpegAvailable READ isFFmpegAvailable NOTIFY ffmpegStatusChanged)
    Q_PROPERTY(QString ffmpegPath READ getFFmpegPath NOTIFY ffmpegStatusChanged)

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

    // === Recording Methods (delegate to ViewModel) ===
    Q_INVOKABLE void startRecording(const QString& mode = "fullscreen");
    Q_INVOKABLE void startRegionRecording(int x, int y, int width, int height);
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void abortRecording();
    Q_INVOKABLE void pauseRecording();
    Q_INVOKABLE void resumeRecording();
    Q_INVOKABLE void convertToGif(const QString& inputPath, const QString& outputPath = QString());

    // === Utility (delegate to ViewModel) ===
    Q_INVOKABLE QString getFFmpegPath();
    Q_INVOKABLE bool isFFmpegAvailable();

    // === File Browser Methods (UI-layer only) ===
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
    void ffmpegStatusChanged();

private slots:
    // ── ViewModel emitter slots ──
    void onVMStateChanged(int state);
    void onVMDurationChanged(int seconds);
    void onVMRecordingCompleted(const QString& filePath);
    void onVMSettingsChanged(const commonHead::viewModels::model::RecordingSettings& settings);
    void onVMError(const QString& message);

private:
    // ViewModel
    std::shared_ptr<commonHead::viewModels::IRecordingViewModel> m_viewModel;
    std::shared_ptr<UIVMSignalEmitter::RecordingViewModelEmitter> m_viewModelEmitter;

    // Shadow state for QML property bindings
    bool m_isRecording = false;
    bool m_isPaused = false;
    int m_recordingDuration = 0;

    // Settings shadow (for QML bindings)
    QString m_outputDirectory;
    QString m_videoFormat = "mp4";
    int m_fps = 30;
};
