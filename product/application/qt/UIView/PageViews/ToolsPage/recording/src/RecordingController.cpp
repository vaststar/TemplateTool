#include "PageViews/ToolsPage/recording/include/RecordingController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <commonHead/viewModels/RecordingViewModel/IRecordingViewModel.h>
#include <commonHead/viewModels/RecordingViewModel/IRecordingModel.h>

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QProcess>
#include <QCoreApplication>
#include <QUrl>

using namespace commonHead::viewModels;
using namespace commonHead::viewModels::model;

RecordingController::RecordingController(QObject* parent)
    : UIViewController(parent)
    , m_viewModelEmitter(std::make_shared<UIVMSignalEmitter::RecordingViewModelEmitter>())
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

// ============================================================================
// init — called after AppContext injection
// ============================================================================

void RecordingController::init()
{
    UIVIEW_LOG_DEBUG("RecordingController::init");

    // Phase 1: Connect emitter Qt signals → controller slots
    using Emitter = UIVMSignalEmitter::RecordingViewModelEmitter;
    connect(m_viewModelEmitter.get(), &Emitter::signals_onSettingsChanged,
            this, &RecordingController::onVMSettingsChanged);
    connect(m_viewModelEmitter.get(), &Emitter::signals_onError,
            this, &RecordingController::onVMError);

    // Phase 2: Create ViewModel from factory
    m_viewModel = getAppContext()->getViewModelFactory()->createRecordingViewModelInstance();
    m_viewModel->initViewModel();

    // Phase 3: Register emitter as callback
    m_viewModel->registerCallback(m_viewModelEmitter);

    // Sync initial settings from ViewModel
    auto settings = m_viewModel->getSettings();
    m_outputDirectory = QString::fromStdString(settings.outputDirectory);
    m_videoFormat = QString::fromStdString(settings.videoFormat);
    m_fps = settings.framesPerSecond;

    // If outputDirectory is not set, use a sensible default
    if (m_outputDirectory.isEmpty()) {
        m_outputDirectory = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
                            + "/Recordings";
        QDir().mkpath(m_outputDirectory);

        // Push default back to ViewModel
        RecordingSettings s = settings;
        s.outputDirectory = m_outputDirectory.toStdString();
        m_viewModel->updateSettings(s);
    }

    m_ffmpegPath = findFFmpegPath();

    // Notify QML that settings properties are now loaded.
    emit settingsChanged();
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
    // FolderDialog returns a QUrl (e.g. "file:///Users/..."), convert to local path
    QString localPath = path;
    if (localPath.startsWith("file://")) {
        localPath = QUrl(localPath).toLocalFile();
    }

    if (m_outputDirectory != localPath) {
        m_outputDirectory = localPath;
        if (m_viewModel) {
            auto s = m_viewModel->getSettings();
            s.outputDirectory = localPath.toStdString();
            m_viewModel->updateSettings(s);
        }
    }
}

void RecordingController::setVideoFormat(const QString& format)
{
    if (m_videoFormat != format) {
        m_videoFormat = format;
        if (m_viewModel) {
            auto s = m_viewModel->getSettings();
            s.videoFormat = format.toStdString();
            m_viewModel->updateSettings(s);
        }
    }
}

void RecordingController::setFps(int fps)
{
    int bounded = qBound(1, fps, 60);
    if (m_fps != bounded) {
        m_fps = bounded;
        if (m_viewModel) {
            auto s = m_viewModel->getSettings();
            s.framesPerSecond = bounded;
            m_viewModel->updateSettings(s);
        }
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
    // On macOS, applicationDirPath() = .../mainEntry.app/Contents/MacOS
    // ffmpeg is in the same directory as the .app bundle (i.e. .../bin/ffmpeg)
    // So we go up 3 levels: MacOS → Contents → mainEntry.app → bin/
    QStringList candidates = {
        appDir + "/../../../ffmpeg",                   // build output: bin/ffmpeg (next to .app)
        appDir + "/ffmpeg",                            // flat layout
        appDir + "/../Resources/ffmpeg",               // bundled inside .app
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
        QString canonicalPath = QFileInfo(path).canonicalFilePath();
        if (!canonicalPath.isEmpty() && QFile::exists(canonicalPath)) {
            UIVIEW_LOG_DEBUG("Found FFmpeg: " << canonicalPath.toStdString());
            return canonicalPath;
        }
    }

    // Fallback: search PATH via 'which'
    QProcess process;
    process.start("which", {"ffmpeg"});
    if (process.waitForFinished(3000) && process.exitCode() == 0) {
        QString path = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        if (!path.isEmpty()) {
            UIVIEW_LOG_DEBUG("Found FFmpeg in PATH: " << path.toStdString());
            return path;
        }
    }

    UIVIEW_LOG_DEBUG("FFmpeg not found. appDir=" << appDir.toStdString());
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

// === ViewModel Callback Slots ===

void RecordingController::onVMSettingsChanged(const RecordingSettings& settings)
{
    bool changed = false;
    QString newDir = QString::fromStdString(settings.outputDirectory);
    QString newFmt = QString::fromStdString(settings.videoFormat);

    if (m_outputDirectory != newDir) { m_outputDirectory = newDir; changed = true; }
    if (m_videoFormat != newFmt) { m_videoFormat = newFmt; changed = true; }
    if (m_fps != settings.framesPerSecond) { m_fps = settings.framesPerSecond; changed = true; }

    if (changed) {
        emit settingsChanged();
    }
}

void RecordingController::onVMError(const QString& message)
{
    UIVIEW_LOG_DEBUG("onVMError: " << message.toStdString());
    emit errorOccurred(message);
}
