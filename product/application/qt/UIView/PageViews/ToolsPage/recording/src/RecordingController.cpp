#include "PageViews/ToolsPage/recording/include/RecordingController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QCoreApplication>
#include <QSettings>

RecordingController::RecordingController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create RecordingController");

    m_recordingTimer = new QTimer(this);
    m_recordingTimer->setInterval(1000);
    connect(m_recordingTimer, &QTimer::timeout, this, &RecordingController::onRecordingDurationTick);
}

RecordingController::~RecordingController()
{
    if (m_isRecording) {
        stopRecording();
    }
}

void RecordingController::init()
{
    UIVIEW_LOG_DEBUG("RecordingController::init");
    loadSettings();
    m_ffmpegPath = findFFmpegPath();
}

// === Property Getters ===

bool RecordingController::isRecording() const
{
    return m_isRecording;
}

int RecordingController::recordingDuration() const
{
    return m_recordingDuration;
}

bool RecordingController::isPaused() const
{
    return m_isPaused;
}

QString RecordingController::outputDirectory() const
{
    return m_outputDirectory;
}

QString RecordingController::videoFormat() const
{
    return m_videoFormat;
}

int RecordingController::fps() const
{
    return m_fps;
}

// === Property Setters ===

void RecordingController::setOutputDirectory(const QString& path)
{
    if (m_outputDirectory != path) {
        m_outputDirectory = path;
        saveSettings();
        emit settingsChanged();
    }
}

void RecordingController::setVideoFormat(const QString& format)
{
    if (m_videoFormat != format) {
        m_videoFormat = format;
        saveSettings();
        emit settingsChanged();
    }
}

void RecordingController::setFps(int fps)
{
    if (m_fps != fps) {
        m_fps = qBound(1, fps, 60);
        saveSettings();
        emit settingsChanged();
    }
}

// === Recording Methods ===

void RecordingController::startRecording(const QString& mode)
{
    if (m_isRecording) {
        emit errorOccurred(tr("Recording already in progress"));
        return;
    }

    if (!isFFmpegAvailable()) {
        emit errorOccurred(tr("FFmpeg not found. Please install FFmpeg to use recording features."));
        return;
    }

    // TODO: Implement FFmpeg-based recording
    UIVIEW_LOG_DEBUG("startRecording: " << mode.toStdString());

    m_isRecording = true;
    m_isPaused = false;
    m_recordingDuration = 0;
    m_recordingTimer->start();

    emit recordingStateChanged();
}

void RecordingController::startRegionRecording(int x, int y, int width, int height)
{
    // TODO: Implement region recording
    Q_UNUSED(x) Q_UNUSED(y) Q_UNUSED(width) Q_UNUSED(height)
    startRecording("region");
}

void RecordingController::stopRecording()
{
    if (!m_isRecording) {
        return;
    }

    UIVIEW_LOG_DEBUG("stopRecording");

    m_recordingTimer->stop();
    m_isRecording = false;
    m_isPaused = false;

    // TODO: Stop FFmpeg process and emit recordingCompleted

    emit recordingStateChanged();
}

void RecordingController::pauseRecording()
{
    if (m_isRecording && !m_isPaused) {
        m_isPaused = true;
        m_recordingTimer->stop();
        emit recordingStateChanged();
    }
}

void RecordingController::resumeRecording()
{
    if (m_isRecording && m_isPaused) {
        m_isPaused = false;
        m_recordingTimer->start();
        emit recordingStateChanged();
    }
}

void RecordingController::onRecordingDurationTick()
{
    m_recordingDuration++;
    emit recordingDurationChanged();
}

void RecordingController::convertToGif(const QString& inputPath, const QString& outputPath)
{
    // TODO: Implement GIF conversion using FFmpeg
    Q_UNUSED(inputPath) Q_UNUSED(outputPath)
    UIVIEW_LOG_DEBUG("convertToGif: not yet implemented");
}

// === Utility ===

QString RecordingController::getFFmpegPath()
{
    return m_ffmpegPath;
}

bool RecordingController::isFFmpegAvailable()
{
    return !m_ffmpegPath.isEmpty() && QFile::exists(m_ffmpegPath);
}

QString RecordingController::findFFmpegPath()
{
    QString appDir = QCoreApplication::applicationDirPath();

#ifdef Q_OS_MAC
    QStringList candidates = {
        appDir + "/ffmpeg",
        appDir + "/../Resources/ffmpeg",
        "/opt/homebrew/bin/ffmpeg",
        "/usr/local/bin/ffmpeg"
    };
#elif defined(Q_OS_WIN)
    QStringList candidates = {
        appDir + "/ffmpeg.exe",
        appDir + "/ffmpeg/ffmpeg.exe"
    };
#else
    QStringList candidates = {
        appDir + "/ffmpeg",
        "/usr/bin/ffmpeg",
        "/usr/local/bin/ffmpeg"
    };
#endif

    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            UIVIEW_LOG_DEBUG("Found FFmpeg: " << path.toStdString());
            return path;
        }
    }

    QProcess process;
    process.start("which", {"ffmpeg"});
    if (process.waitForFinished(3000) && process.exitCode() == 0) {
        QString path = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        if (!path.isEmpty()) {
            UIVIEW_LOG_DEBUG("Found FFmpeg in PATH: " << path.toStdString());
            return path;
        }
    }

    UIVIEW_LOG_DEBUG("FFmpeg not found");
    return QString();
}

QString RecordingController::getDefaultSavePath()
{
    if (!m_outputDirectory.isEmpty() && QDir(m_outputDirectory).exists()) {
        return m_outputDirectory;
    }
    return QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
}

// === File Browser Methods ===

void RecordingController::openRecordingsFolder()
{
    UIVIEW_LOG_DEBUG("openRecordingsFolder: " << m_outputDirectory.toStdString());

    if (m_outputDirectory.isEmpty()) {
        emit errorOccurred(tr("Recordings folder not set"));
        return;
    }

#ifdef Q_OS_MAC
    QProcess::startDetached("open", {m_outputDirectory});
#elif defined(Q_OS_WIN)
    QProcess::startDetached("explorer", {QDir::toNativeSeparators(m_outputDirectory)});
#elif defined(Q_OS_LINUX)
    QProcess::startDetached("xdg-open", {m_outputDirectory});
#endif
}

void RecordingController::openFile(const QString& filePath)
{
    UIVIEW_LOG_DEBUG("openFile: " << filePath.toStdString());

    if (!QFile::exists(filePath)) {
        emit errorOccurred(tr("File not found: %1").arg(filePath));
        return;
    }

#ifdef Q_OS_MAC
    QProcess::startDetached("open", {filePath});
#elif defined(Q_OS_WIN)
    QProcess::startDetached("cmd", {"/c", "start", "", QDir::toNativeSeparators(filePath)});
#elif defined(Q_OS_LINUX)
    QProcess::startDetached("xdg-open", {filePath});
#endif
}

// === Settings ===

void RecordingController::loadSettings()
{
    QSettings settings;
    settings.beginGroup("ScreenRecording");

    QString defaultFolder = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
                            + "/Recordings";
    m_outputDirectory = settings.value("outputDirectory", defaultFolder).toString();
    QDir().mkpath(m_outputDirectory);

    m_videoFormat = settings.value("videoFormat", "mp4").toString();
    m_fps = settings.value("fps", 30).toInt();
    settings.endGroup();
}

void RecordingController::saveSettings()
{
    QSettings settings;
    settings.beginGroup("ScreenRecording");
    settings.setValue("outputDirectory", m_outputDirectory);
    settings.setValue("videoFormat", m_videoFormat);
    settings.setValue("fps", m_fps);
    settings.endGroup();
}
