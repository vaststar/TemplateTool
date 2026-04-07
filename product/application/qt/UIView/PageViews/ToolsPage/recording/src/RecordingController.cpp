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
#include <QProcess>
#include <QCoreApplication>
#include <QUrl>

using namespace commonHead::viewModels;
using namespace commonHead::viewModels::model;

// ============================================================================
// Construction
// ============================================================================

RecordingController::RecordingController(QObject* parent)
    : UIViewController(parent)
    , m_viewModelEmitter(std::make_shared<UIVMSignalEmitter::RecordingViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create RecordingController");
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
    connect(m_viewModelEmitter.get(), &Emitter::signals_onStateChanged,
            this, &RecordingController::onVMStateChanged);
    connect(m_viewModelEmitter.get(), &Emitter::signals_onDurationChanged,
            this, &RecordingController::onVMDurationChanged);
    connect(m_viewModelEmitter.get(), &Emitter::signals_onRecordingCompleted,
            this, &RecordingController::onVMRecordingCompleted);
    connect(m_viewModelEmitter.get(), &Emitter::signals_onSettingsChanged,
            this, &RecordingController::onVMSettingsChanged);
    connect(m_viewModelEmitter.get(), &Emitter::signals_onError,
            this, &RecordingController::onVMError);

    // Phase 2: Create ViewModel from factory
    m_viewModel = getAppContext()->getViewModelFactory()->createRecordingViewModelInstance();
    m_viewModel->initViewModel();

    // Phase 3: Register emitter as callback
    m_viewModel->registerCallback(m_viewModelEmitter);

    // Phase 4: Provide appDir to ViewModel for FFmpeg discovery
    std::string appDir = QCoreApplication::applicationDirPath().toStdString();
    m_viewModel->setAppDir(appDir);

    // Notify QML that FFmpeg status is now resolved
    emit ffmpegStatusChanged();

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

    // Notify QML that settings properties are now loaded.
    emit settingsChanged();
}

// ============================================================================
// Property Getters
// ============================================================================

bool RecordingController::isRecording() const { return m_isRecording; }
int RecordingController::recordingDuration() const { return m_recordingDuration; }
bool RecordingController::isPaused() const { return m_isPaused; }
QString RecordingController::outputDirectory() const { return m_outputDirectory; }

QString RecordingController::recordingsFolderUrl() const
{
    return QUrl::fromLocalFile(
        (!m_outputDirectory.isEmpty() && QDir(m_outputDirectory).exists())
            ? m_outputDirectory
            : QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
    ).toString();
}

QString RecordingController::videoFormat() const { return m_videoFormat; }
int RecordingController::fps() const { return m_fps; }

// ============================================================================
// Property Setters — push to ViewModel
// ============================================================================

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

// ============================================================================
// Recording Methods — delegate to ViewModel
// ============================================================================

void RecordingController::startRecording(const QString& mode)
{
    Q_UNUSED(mode)
    if (m_viewModel) {
        m_viewModel->startRecording();
    }
}

void RecordingController::startRegionRecording(int x, int y, int width, int height)
{
    if (m_viewModel) {
        m_viewModel->startRegionRecording(x, y, width, height);
    }
}

void RecordingController::stopRecording()
{
    if (m_viewModel) {
        m_viewModel->stopRecording();
    }
}

void RecordingController::abortRecording()
{
    if (m_viewModel) {
        m_viewModel->abortRecording();
    }
}

void RecordingController::pauseRecording()
{
    if (m_viewModel) {
        m_viewModel->pauseRecording();
    }
}

void RecordingController::resumeRecording()
{
    if (m_viewModel) {
        m_viewModel->resumeRecording();
    }
}

void RecordingController::convertToGif(const QString& inputPath, const QString& outputPath)
{
    if (m_viewModel) {
        m_viewModel->convertToGif(inputPath.toStdString(), outputPath.toStdString());
    }
}

// ============================================================================
// Utility — delegate to ViewModel
// ============================================================================

QString RecordingController::getFFmpegPath()
{
    if (m_viewModel) {
        return QString::fromStdString(m_viewModel->getFFmpegPath());
    }
    return {};
}

bool RecordingController::isFFmpegAvailable()
{
    if (m_viewModel) {
        return m_viewModel->isFFmpegAvailable();
    }
    return false;
}

QString RecordingController::getDefaultSavePath()
{
    if (!m_outputDirectory.isEmpty() && QDir(m_outputDirectory).exists()) {
        return m_outputDirectory;
    }
    return QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
}

// ============================================================================
// File Browser Methods (UI-layer only — uses platform Qt APIs)
// ============================================================================

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

// ============================================================================
// ViewModel Callback Slots
// ============================================================================

void RecordingController::onVMStateChanged(int state)
{
    auto newState = static_cast<RecordingState>(state);
    bool wasRecording = m_isRecording;
    bool wasPaused = m_isPaused;

    m_isRecording = (newState == RecordingState::Recording || newState == RecordingState::Paused);
    m_isPaused = (newState == RecordingState::Paused);

    if (newState == RecordingState::Idle) {
        m_recordingDuration = 0;
    }

    if (m_isRecording != wasRecording || m_isPaused != wasPaused) {
        emit recordingStateChanged();
    }
}

void RecordingController::onVMDurationChanged(int seconds)
{
    if (m_recordingDuration != seconds) {
        m_recordingDuration = seconds;
        emit recordingDurationChanged();
    }
}

void RecordingController::onVMRecordingCompleted(const QString& filePath)
{
    UIVIEW_LOG_DEBUG("onVMRecordingCompleted: " << filePath.toStdString());
    emit recordingCompleted(filePath);
}

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
