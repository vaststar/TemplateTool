#include "PageViews/ToolsPage/screencapture/include/ScreenCaptureController.h"
#include "PageViews/ToolsPage/screencapture/include/ScreenshotService.h"
#include "LoggerDefine/LoggerDefine.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPolygon>
#include <QSettings>
#include <QProcess>
#include <QBuffer>
#include <QMimeData>
#include <cmath>

ScreenCaptureController::ScreenCaptureController(QObject* parent)
    : UIViewController(parent)
    , m_screenshotService(std::make_unique<ScreenshotService>())
{
    UIVIEW_LOG_DEBUG("create ScreenCaptureController");

    m_delayTimer = new QTimer(this);
    m_delayTimer->setInterval(1000);
    connect(m_delayTimer, &QTimer::timeout, this, &ScreenCaptureController::onDelayTimerTick);

    m_recordingTimer = new QTimer(this);
    m_recordingTimer->setInterval(1000);
    connect(m_recordingTimer, &QTimer::timeout, this, &ScreenCaptureController::onRecordingDurationTick);
}

ScreenCaptureController::~ScreenCaptureController()
{
    if (m_isRecording) {
        stopRecording();
    }
}

void ScreenCaptureController::init()
{
    UIVIEW_LOG_DEBUG("ScreenCaptureController::init");
    loadSettings();
    m_ffmpegPath = findFFmpegPath();
}

// === Property Getters ===

QImage ScreenCaptureController::currentScreenshot() const
{
    return m_currentScreenshot;
}

bool ScreenCaptureController::hasScreenshot() const
{
    return !m_currentScreenshot.isNull();
}

QString ScreenCaptureController::screenshotBase64() const
{
    if (m_currentScreenshot.isNull()) {
        return QString();
    }

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    m_currentScreenshot.save(&buffer, "PNG");
    return QString::fromLatin1(byteArray.toBase64());
}

int ScreenCaptureController::screenshotWidth() const
{
    return m_currentScreenshot.width();
}

int ScreenCaptureController::screenshotHeight() const
{
    return m_currentScreenshot.height();
}

QVariantList ScreenCaptureController::annotations() const
{
    return m_annotations;
}

bool ScreenCaptureController::isRecording() const
{
    return m_isRecording;
}

int ScreenCaptureController::recordingDuration() const
{
    return m_recordingDuration;
}

bool ScreenCaptureController::isPaused() const
{
    return m_isPaused;
}

QString ScreenCaptureController::outputDirectory() const
{
    return m_outputDirectory;
}

QString ScreenCaptureController::screenshotsFolderUrl() const
{
    return QStringLiteral("file://") + m_outputDirectory;
}

int ScreenCaptureController::delaySeconds() const
{
    return m_delaySeconds;
}

QString ScreenCaptureController::imageFormat() const
{
    return m_imageFormat;
}

QString ScreenCaptureController::videoFormat() const
{
    return m_videoFormat;
}

bool ScreenCaptureController::includeTimestamp() const
{
    return m_includeTimestamp;
}

int ScreenCaptureController::fps() const
{
    return m_fps;
}

// === Property Setters ===

void ScreenCaptureController::setOutputDirectory(const QString& path)
{
    if (m_outputDirectory != path) {
        m_outputDirectory = path;
        saveSettings();
        emit settingsChanged();
    }
}

void ScreenCaptureController::setDelaySeconds(int seconds)
{
    if (m_delaySeconds != seconds) {
        m_delaySeconds = seconds;
        saveSettings();
        emit settingsChanged();
    }
}

void ScreenCaptureController::setImageFormat(const QString& format)
{
    if (m_imageFormat != format) {
        m_imageFormat = format;
        saveSettings();
        emit settingsChanged();
    }
}

void ScreenCaptureController::setVideoFormat(const QString& format)
{
    if (m_videoFormat != format) {
        m_videoFormat = format;
        saveSettings();
        emit settingsChanged();
    }
}

void ScreenCaptureController::setIncludeTimestamp(bool include)
{
    if (m_includeTimestamp != include) {
        m_includeTimestamp = include;
        saveSettings();
        emit settingsChanged();
    }
}

void ScreenCaptureController::setFps(int fps)
{
    if (m_fps != fps) {
        m_fps = qBound(1, fps, 60);
        saveSettings();
        emit settingsChanged();
    }
}

// === Screenshot Methods ===

void ScreenCaptureController::captureFullScreen()
{
    UIVIEW_LOG_DEBUG("captureFullScreen");

    // Ensure output directory exists
    if (m_outputDirectory.isEmpty()) {
        m_outputDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                            + "/Screenshots";
    }
    QDir().mkpath(m_outputDirectory);

    // Generate unique filename
    QString filename = generateFilename(m_imageFormat);
    QString filePath = m_outputDirectory + "/" + filename;

    // Capture screen
    QImage screenshot = ScreenshotService::captureFullScreen();

    if (screenshot.isNull()) {
        emit errorOccurred(tr("Failed to capture screen"));
        return;
    }

    if (m_includeTimestamp) {
        screenshot = ScreenshotService::addTimestamp(screenshot);
    }

    // Save to file
    if (screenshot.save(filePath)) {
        UIVIEW_LOG_DEBUG("Screenshot saved to: " << filePath.toStdString());
        emit captureCompleted(filePath);

        // Open in Preview for editing
#ifdef Q_OS_MAC
        QProcess::startDetached("open", {"-a", "Preview", filePath});
#elif defined(Q_OS_WIN)
        QProcess::startDetached("cmd", {"/c", "start", "", QDir::toNativeSeparators(filePath)});
#elif defined(Q_OS_LINUX)
        QProcess::startDetached("xdg-open", {filePath});
#endif
    } else {
        emit errorOccurred(tr("Failed to save screenshot"));
    }
}

void ScreenCaptureController::captureRegion(int x, int y, int width, int height)
{
    UIVIEW_LOG_DEBUG("captureRegion: " << x << "," << y << " " << width << "x" << height);

    QRect region(x, y, width, height);
    m_currentScreenshot = ScreenshotService::captureRegion(region);

    if (m_includeTimestamp) {
        m_currentScreenshot = ScreenshotService::addTimestamp(m_currentScreenshot);
    }

    m_annotations.clear();
    m_undoStack.clear();
    m_redoStack.clear();

    emit screenshotChanged();
    emit annotationsChanged();
}

void ScreenCaptureController::captureInteractiveRegion()
{
    UIVIEW_LOG_DEBUG("captureInteractiveRegion - using system tool");

    // Ensure output directory exists
    if (m_outputDirectory.isEmpty()) {
        m_outputDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                            + "/Screenshots";
    }
    QDir().mkpath(m_outputDirectory);

    // Generate unique filename
    QString filename = generateFilename(m_imageFormat);
    QString filePath = m_outputDirectory + "/" + filename;

#ifdef Q_OS_MAC
    // Use macOS screencapture tool with interactive selection
    // -i: interactive mode (select region or window)
    // -x: no sound
    // Save directly to file, then open in Preview for editing
    QProcess process;
    process.start("screencapture", {"-i", "-x", filePath});
    process.waitForFinished(-1);  // Wait indefinitely for user selection

    if (process.exitCode() == 0 && QFile::exists(filePath)) {
        UIVIEW_LOG_DEBUG("Screenshot saved to: " << filePath.toStdString());
        emit captureCompleted(filePath);

        // Open in Preview for editing
        QProcess::startDetached("open", {"-a", "Preview", filePath});
    } else {
        UIVIEW_LOG_DEBUG("screencapture cancelled or failed");
    }
#elif defined(Q_OS_WIN)
    // Windows: Use Snipping Tool
    // For now, capture full screen and save
    m_currentScreenshot = ScreenshotService::captureFullScreen();
    if (!m_currentScreenshot.isNull()) {
        m_currentScreenshot.save(filePath);
        emit captureCompleted(filePath);
        // Open with default viewer
        QProcess::startDetached("cmd", {"/c", "start", "", QDir::toNativeSeparators(filePath)});
    }
#elif defined(Q_OS_LINUX)
    // Try gnome-screenshot
    QProcess process;
    process.start("gnome-screenshot", {"-a", "-f", filePath});
    if (!process.waitForStarted(1000)) {
        // Try scrot
        process.start("scrot", {"-s", filePath});
    }
    process.waitForFinished(-1);
    if (process.exitCode() == 0 && QFile::exists(filePath)) {
        emit captureCompleted(filePath);
        // Open with default viewer
        QProcess::startDetached("xdg-open", {filePath});
    }
#else
    emit errorOccurred(tr("Interactive region capture not supported on this platform."));
#endif
}

void ScreenCaptureController::loadFromClipboard()
{
    UIVIEW_LOG_DEBUG("loadFromClipboard");

    QClipboard* clipboard = QGuiApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();

    if (mimeData->hasImage()) {
        m_currentScreenshot = qvariant_cast<QImage>(mimeData->imageData());

        if (m_includeTimestamp) {
            m_currentScreenshot = ScreenshotService::addTimestamp(m_currentScreenshot);
        }

        m_annotations.clear();
        m_undoStack.clear();
        m_redoStack.clear();

        emit screenshotChanged();
        emit annotationsChanged();

        UIVIEW_LOG_DEBUG("Loaded image from clipboard: " << m_currentScreenshot.width() << "x" << m_currentScreenshot.height());
    } else {
        UIVIEW_LOG_DEBUG("No image in clipboard");
        emit errorOccurred(tr("No image found in clipboard"));
    }
}

void ScreenCaptureController::captureWithDelay(int seconds, const QString& mode)
{
    UIVIEW_LOG_DEBUG("captureWithDelay: " << seconds << "s, mode=" << mode.toStdString());

    m_delayCountdown = seconds;
    m_pendingCaptureMode = mode;

    if (seconds > 0) {
        emit delayCountdown(m_delayCountdown);
        m_delayTimer->start();
    } else {
        if (mode == "fullscreen") {
            captureFullScreen();
        }
    }
}

void ScreenCaptureController::onDelayTimerTick()
{
    m_delayCountdown--;
    emit delayCountdown(m_delayCountdown);

    if (m_delayCountdown <= 0) {
        m_delayTimer->stop();

        if (m_pendingCaptureMode == "fullscreen") {
            captureFullScreen();
        } else if (m_pendingCaptureMode == "region") {
            captureRegion(m_pendingRegion.x(), m_pendingRegion.y(),
                          m_pendingRegion.width(), m_pendingRegion.height());
        }
    }
}

QVariantList ScreenCaptureController::getWindowList()
{
    return ScreenshotService::getWindowListAsVariant();
}

void ScreenCaptureController::captureWindow(qint64 windowId)
{
    UIVIEW_LOG_DEBUG("captureWindow: " << windowId);

    // Ensure output directory exists
    if (m_outputDirectory.isEmpty()) {
        m_outputDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                            + "/Screenshots";
    }
    QDir().mkpath(m_outputDirectory);

    // Generate unique filename
    QString filename = generateFilename(m_imageFormat);
    QString filePath = m_outputDirectory + "/" + filename;

    // Capture window
    QImage screenshot = ScreenshotService::captureWindow(windowId);

    if (screenshot.isNull()) {
        emit errorOccurred(tr("Failed to capture window"));
        return;
    }

    if (m_includeTimestamp) {
        screenshot = ScreenshotService::addTimestamp(screenshot);
    }

    // Save to file
    if (screenshot.save(filePath)) {
        UIVIEW_LOG_DEBUG("Screenshot saved to: " << filePath.toStdString());
        emit captureCompleted(filePath);

        // Open in Preview for editing
#ifdef Q_OS_MAC
        QProcess::startDetached("open", {"-a", "Preview", filePath});
#elif defined(Q_OS_WIN)
        QProcess::startDetached("cmd", {"/c", "start", "", QDir::toNativeSeparators(filePath)});
#elif defined(Q_OS_LINUX)
        QProcess::startDetached("xdg-open", {filePath});
#endif
    } else {
        emit errorOccurred(tr("Failed to save screenshot"));
    }
}

// === Editing Methods ===

void ScreenCaptureController::addRectangle(int x, int y, int w, int h, const QString& color, int thickness)
{
    QVariantMap annotation;
    annotation["type"] = "rectangle";
    annotation["x"] = x;
    annotation["y"] = y;
    annotation["width"] = w;
    annotation["height"] = h;
    annotation["color"] = color;
    annotation["thickness"] = thickness;

    m_undoStack.append(m_annotations);
    m_redoStack.clear();
    m_annotations.append(annotation);

    emit annotationsChanged();
}

void ScreenCaptureController::addEllipse(int x, int y, int w, int h, const QString& color, int thickness)
{
    QVariantMap annotation;
    annotation["type"] = "ellipse";
    annotation["x"] = x;
    annotation["y"] = y;
    annotation["width"] = w;
    annotation["height"] = h;
    annotation["color"] = color;
    annotation["thickness"] = thickness;

    m_undoStack.append(m_annotations);
    m_redoStack.clear();
    m_annotations.append(annotation);

    emit annotationsChanged();
}

void ScreenCaptureController::addArrow(int x1, int y1, int x2, int y2, const QString& color, int thickness)
{
    QVariantMap annotation;
    annotation["type"] = "arrow";
    annotation["x1"] = x1;
    annotation["y1"] = y1;
    annotation["x2"] = x2;
    annotation["y2"] = y2;
    annotation["color"] = color;
    annotation["thickness"] = thickness;

    m_undoStack.append(m_annotations);
    m_redoStack.clear();
    m_annotations.append(annotation);

    emit annotationsChanged();
}

void ScreenCaptureController::addText(int x, int y, const QString& text, const QString& color, int fontSize)
{
    QVariantMap annotation;
    annotation["type"] = "text";
    annotation["x"] = x;
    annotation["y"] = y;
    annotation["text"] = text;
    annotation["color"] = color;
    annotation["fontSize"] = fontSize;

    m_undoStack.append(m_annotations);
    m_redoStack.clear();
    m_annotations.append(annotation);

    emit annotationsChanged();
}

void ScreenCaptureController::addFreehand(const QVariantList& points, const QString& color, int thickness)
{
    QVariantMap annotation;
    annotation["type"] = "freehand";
    annotation["points"] = points;
    annotation["color"] = color;
    annotation["thickness"] = thickness;

    m_undoStack.append(m_annotations);
    m_redoStack.clear();
    m_annotations.append(annotation);

    emit annotationsChanged();
}

void ScreenCaptureController::addMosaic(int x, int y, int w, int h, int blockSize)
{
    QVariantMap annotation;
    annotation["type"] = "mosaic";
    annotation["x"] = x;
    annotation["y"] = y;
    annotation["width"] = w;
    annotation["height"] = h;
    annotation["blockSize"] = blockSize;

    m_undoStack.append(m_annotations);
    m_redoStack.clear();
    m_annotations.append(annotation);

    emit annotationsChanged();
}

void ScreenCaptureController::undo()
{
    if (!m_undoStack.isEmpty()) {
        m_redoStack.append(m_annotations);
        m_annotations = m_undoStack.takeLast().toList();
        emit annotationsChanged();
    }
}

void ScreenCaptureController::redo()
{
    if (!m_redoStack.isEmpty()) {
        m_undoStack.append(m_annotations);
        m_annotations = m_redoStack.takeLast().toList();
        emit annotationsChanged();
    }
}

void ScreenCaptureController::clearAnnotations()
{
    if (!m_annotations.isEmpty()) {
        m_undoStack.append(m_annotations);
        m_redoStack.clear();
        m_annotations.clear();
        emit annotationsChanged();
    }
}

// === Save/Export Methods ===

QImage ScreenCaptureController::renderAnnotations(const QImage& base)
{
    QImage result = base.copy();
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const QVariant& var : m_annotations) {
        QVariantMap ann = var.toMap();
        QString type = ann["type"].toString();
        QColor color(ann["color"].toString());
        int thickness = ann["thickness"].toInt();

        QPen pen(color, thickness);
        painter.setPen(pen);

        if (type == "rectangle") {
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(ann["x"].toInt(), ann["y"].toInt(),
                           ann["width"].toInt(), ann["height"].toInt());
        }
        else if (type == "ellipse") {
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(ann["x"].toInt(), ann["y"].toInt(),
                              ann["width"].toInt(), ann["height"].toInt());
        }
        else if (type == "arrow") {
            int x1 = ann["x1"].toInt(), y1 = ann["y1"].toInt();
            int x2 = ann["x2"].toInt(), y2 = ann["y2"].toInt();
            painter.drawLine(x1, y1, x2, y2);

            // Draw arrowhead
            double angle = std::atan2(y2 - y1, x2 - x1);
            int arrowSize = 10 + thickness;
            QPointF p1(x2 - arrowSize * std::cos(angle - M_PI/6),
                      y2 - arrowSize * std::sin(angle - M_PI/6));
            QPointF p2(x2 - arrowSize * std::cos(angle + M_PI/6),
                      y2 - arrowSize * std::sin(angle + M_PI/6));
            painter.setBrush(color);
            painter.drawPolygon(QPolygonF() << QPointF(x2, y2) << p1 << p2);
        }
        else if (type == "text") {
            QFont font;
            font.setPixelSize(ann["fontSize"].toInt());
            painter.setFont(font);
            painter.drawText(ann["x"].toInt(), ann["y"].toInt(), ann["text"].toString());
        }
        else if (type == "freehand") {
            QVariantList points = ann["points"].toList();
            if (points.size() >= 2) {
                QPainterPath path;
                QVariantMap p0 = points[0].toMap();
                path.moveTo(p0["x"].toDouble(), p0["y"].toDouble());
                for (int i = 1; i < points.size(); ++i) {
                    QVariantMap p = points[i].toMap();
                    path.lineTo(p["x"].toDouble(), p["y"].toDouble());
                }
                painter.setBrush(Qt::NoBrush);
                painter.drawPath(path);
            }
        }
        else if (type == "mosaic") {
            int mx = ann["x"].toInt();
            int my = ann["y"].toInt();
            int mw = ann["width"].toInt();
            int mh = ann["height"].toInt();
            int blockSize = ann["blockSize"].toInt();

            // Apply mosaic effect
            for (int by = my; by < my + mh; by += blockSize) {
                for (int bx = mx; bx < mx + mw; bx += blockSize) {
                    int avgR = 0, avgG = 0, avgB = 0, count = 0;
                    for (int py = by; py < by + blockSize && py < my + mh && py < result.height(); ++py) {
                        for (int px = bx; px < bx + blockSize && px < mx + mw && px < result.width(); ++px) {
                            QColor c = result.pixelColor(px, py);
                            avgR += c.red();
                            avgG += c.green();
                            avgB += c.blue();
                            count++;
                        }
                    }
                    if (count > 0) {
                        QColor avgColor(avgR / count, avgG / count, avgB / count);
                        for (int py = by; py < by + blockSize && py < my + mh && py < result.height(); ++py) {
                            for (int px = bx; px < bx + blockSize && px < mx + mw && px < result.width(); ++px) {
                                result.setPixelColor(px, py, avgColor);
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

bool ScreenCaptureController::saveScreenshot(const QString& filePath)
{
    if (m_currentScreenshot.isNull()) {
        emit errorOccurred(tr("No screenshot to save"));
        return false;
    }

    QString path = filePath;
    if (path.isEmpty()) {
        path = getDefaultSavePath() + "/" + generateFilename(m_imageFormat);
    }

    QImage finalImage = renderAnnotations(m_currentScreenshot);

    if (finalImage.save(path)) {
        UIVIEW_LOG_DEBUG("Screenshot saved to: " << path.toStdString());
        emit captureCompleted(path);
        return true;
    } else {
        emit errorOccurred(tr("Failed to save screenshot"));
        return false;
    }
}

void ScreenCaptureController::copyToClipboard()
{
    if (m_currentScreenshot.isNull()) {
        emit errorOccurred(tr("No screenshot to copy"));
        return;
    }

    QImage finalImage = renderAnnotations(m_currentScreenshot);
    QGuiApplication::clipboard()->setImage(finalImage);
    UIVIEW_LOG_DEBUG("Screenshot copied to clipboard");
}

QString ScreenCaptureController::generateFilename(const QString& extension)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    return QString("screenshot_%1.%2").arg(timestamp, extension);
}

QString ScreenCaptureController::getDefaultSavePath()
{
    if (!m_outputDirectory.isEmpty() && QDir(m_outputDirectory).exists()) {
        return m_outputDirectory;
    }
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

void ScreenCaptureController::discardScreenshot()
{
    m_currentScreenshot = QImage();
    m_annotations.clear();
    m_undoStack.clear();
    m_redoStack.clear();
    emit screenshotChanged();
    emit annotationsChanged();
}

// === Recording Methods ===

void ScreenCaptureController::startRecording(const QString& mode)
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

void ScreenCaptureController::startRegionRecording(int x, int y, int width, int height)
{
    // TODO: Implement region recording
    Q_UNUSED(x) Q_UNUSED(y) Q_UNUSED(width) Q_UNUSED(height)
    startRecording("region");
}

void ScreenCaptureController::stopRecording()
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

void ScreenCaptureController::pauseRecording()
{
    if (m_isRecording && !m_isPaused) {
        m_isPaused = true;
        m_recordingTimer->stop();
        emit recordingStateChanged();
    }
}

void ScreenCaptureController::resumeRecording()
{
    if (m_isRecording && m_isPaused) {
        m_isPaused = false;
        m_recordingTimer->start();
        emit recordingStateChanged();
    }
}

void ScreenCaptureController::onRecordingDurationTick()
{
    m_recordingDuration++;
    emit recordingDurationChanged();
}

void ScreenCaptureController::convertToGif(const QString& inputPath, const QString& outputPath)
{
    // TODO: Implement GIF conversion using FFmpeg
    Q_UNUSED(inputPath) Q_UNUSED(outputPath)
    UIVIEW_LOG_DEBUG("convertToGif: not yet implemented");
}

// === Utility ===

QString ScreenCaptureController::getFFmpegPath()
{
    return m_ffmpegPath;
}

bool ScreenCaptureController::isFFmpegAvailable()
{
    return !m_ffmpegPath.isEmpty() && QFile::exists(m_ffmpegPath);
}

QString ScreenCaptureController::findFFmpegPath()
{
    // Check bundled FFmpeg first
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

    // Try system PATH
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

void ScreenCaptureController::loadSettings()
{
    QSettings settings;
    settings.beginGroup("ScreenCapture");

    // Default to Pictures/Screenshots folder
    QString defaultFolder = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                            + "/Screenshots";
    m_outputDirectory = settings.value("outputDirectory", defaultFolder).toString();

    // Ensure folder exists
    QDir().mkpath(m_outputDirectory);

    m_delaySeconds = settings.value("delaySeconds", 0).toInt();
    m_imageFormat = settings.value("imageFormat", "png").toString();
    m_videoFormat = settings.value("videoFormat", "mp4").toString();
    m_includeTimestamp = settings.value("includeTimestamp", false).toBool();  // Default off for cleaner files
    m_fps = settings.value("fps", 30).toInt();
    settings.endGroup();
}

void ScreenCaptureController::saveSettings()
{
    QSettings settings;
    settings.beginGroup("ScreenCapture");
    settings.setValue("outputDirectory", m_outputDirectory);
    settings.setValue("delaySeconds", m_delaySeconds);
    settings.setValue("imageFormat", m_imageFormat);
    settings.setValue("videoFormat", m_videoFormat);
    settings.setValue("includeTimestamp", m_includeTimestamp);
    settings.setValue("fps", m_fps);
    settings.endGroup();
}

// === File Browser Methods ===

void ScreenCaptureController::openScreenshotsFolder()
{
    UIVIEW_LOG_DEBUG("openScreenshotsFolder: " << m_outputDirectory.toStdString());

    if (m_outputDirectory.isEmpty()) {
        emit errorOccurred(tr("Screenshots folder not set"));
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

void ScreenCaptureController::openFile(const QString& filePath)
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

void ScreenCaptureController::copyFileToClipboard(const QString& filePath)
{
    UIVIEW_LOG_DEBUG("copyFileToClipboard: " << filePath.toStdString());

    QImage image(filePath);
    if (image.isNull()) {
        emit errorOccurred(tr("Failed to load image: %1").arg(filePath));
        return;
    }

    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setImage(image);
}

void ScreenCaptureController::revealInFinder(const QString& filePath)
{
    UIVIEW_LOG_DEBUG("revealInFinder: " << filePath.toStdString());

    if (!QFile::exists(filePath)) {
        emit errorOccurred(tr("File not found: %1").arg(filePath));
        return;
    }

#ifdef Q_OS_MAC
    QProcess::startDetached("open", {"-R", filePath});
#elif defined(Q_OS_WIN)
    QProcess::startDetached("explorer", {"/select,", QDir::toNativeSeparators(filePath)});
#elif defined(Q_OS_LINUX)
    // Open containing folder
    QFileInfo fi(filePath);
    QProcess::startDetached("xdg-open", {fi.absolutePath()});
#endif
}

void ScreenCaptureController::deleteFile(const QString& filePath)
{
    UIVIEW_LOG_DEBUG("deleteFile: " << filePath.toStdString());

    if (!QFile::exists(filePath)) {
        emit errorOccurred(tr("File not found: %1").arg(filePath));
        return;
    }

    if (QFile::remove(filePath)) {
        UIVIEW_LOG_DEBUG("File deleted successfully");
    } else {
        emit errorOccurred(tr("Failed to delete file: %1").arg(filePath));
    }
}

// === Overlay Screenshot Methods ===

QVariantMap ScreenCaptureController::grabScreenForOverlay()
{
    UIVIEW_LOG_DEBUG("grabScreenForOverlay");

    QVariantMap result;

    // Capture full screen
    QImage screenshot = ScreenshotService::captureFullScreen();

    if (screenshot.isNull()) {
        result["success"] = false;
        result["error"] = tr("Failed to capture screen");
        return result;
    }

    // Convert to base64
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    screenshot.save(&buffer, "PNG");

    // Store for later use
    m_currentScreenshot = screenshot;

    result["success"] = true;
    result["base64"] = QString::fromLatin1(byteArray.toBase64());
    result["width"] = screenshot.width();
    result["height"] = screenshot.height();

    UIVIEW_LOG_DEBUG("Screen captured: " << screenshot.width() << "x" << screenshot.height());

    return result;
}

QVariantMap ScreenCaptureController::saveRegionScreenshot(int x, int y, int w, int h, int windowWidth, int windowHeight, const QVariantList& annotations)
{
    QVariantMap result;

    if (m_currentScreenshot.isNull()) {
        result["success"] = false;
        result["error"] = tr("No screenshot available");
        return result;
    }

    // windowWidth/windowHeight are the painted dimensions of the image in QML
    // Calculate scale factor from actual image size vs painted size
    qreal scaleX = static_cast<qreal>(m_currentScreenshot.width()) / windowWidth;
    qreal scaleY = static_cast<qreal>(m_currentScreenshot.height()) / windowHeight;
    // For PreserveAspectFit, both scales should be approximately equal
    qreal scale = (scaleX + scaleY) / 2.0;

    // Scale coordinates to physical pixels
    int physX = qRound(x * scale);
    int physY = qRound(y * scale);
    int physW = qRound(w * scale);
    int physH = qRound(h * scale);

    // Clamp to image bounds - adjust width/height when position is clamped
    if (physX < 0) {
        physW += physX;  // Reduce width by the amount physX is negative
        physX = 0;
    }
    if (physY < 0) {
        physH += physY;  // Reduce height by the amount physY is negative
        physY = 0;
    }
    if (physX + physW > m_currentScreenshot.width()) {
        physW = m_currentScreenshot.width() - physX;
    }
    if (physY + physH > m_currentScreenshot.height()) {
        physH = m_currentScreenshot.height() - physY;
    }

    // Ensure valid dimensions
    if (physW <= 0 || physH <= 0) {
        result["success"] = false;
        result["error"] = tr("Invalid selection region");
        return result;
    }

    // Ensure output directory exists
    if (m_outputDirectory.isEmpty()) {
        m_outputDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                            + "/Screenshots";
    }
    QDir().mkpath(m_outputDirectory);

    // Crop the region using physical pixel coordinates
    QImage croppedRaw = m_currentScreenshot.copy(physX, physY, physW, physH);
    if (croppedRaw.isNull()) {
        result["success"] = false;
        result["error"] = tr("Failed to crop region");
        return result;
    }

    // Convert to ARGB32 for painting, reset devicePixelRatio to 1
    // and call bits() to force detach (deep copy)
    QImage cropped = croppedRaw.convertToFormat(QImage::Format_ARGB32);
    cropped.setDevicePixelRatio(1.0);  // Reset to 1 so painting coordinates match pixel coordinates
    cropped.bits();  // Force detach to ensure we have an independent copy

    // Calculate annotation scale (cropped image size / QML selection size)
    qreal annScaleX = static_cast<qreal>(cropped.width()) / w;
    qreal annScaleY = static_cast<qreal>(cropped.height()) / h;
    qreal annScale = (annScaleX + annScaleY) / 2.0;

    // Draw annotations on the cropped image (scale coordinates)
    if (!annotations.isEmpty()) {
        QPainter painter(&cropped);
        if (!painter.isActive()) {
            result["success"] = false;
            result["error"] = tr("Failed to create painter");
            return result;
        }
        painter.setRenderHint(QPainter::Antialiasing);

        for (int annIdx = 0; annIdx < annotations.size(); ++annIdx) {
            QVariantMap ann = annotations[annIdx].toMap();
            QString type = ann["type"].toString();
            QString colorStr = ann["color"].toString();
            int rawThickness = ann["thickness"].toInt();
            int thickness = qMax(1, qRound(rawThickness * annScale));

            QColor annColor(colorStr);
            if (!annColor.isValid()) {
                annColor = QColor(Qt::red);
            }

            QPen pen;
            pen.setColor(annColor);
            pen.setWidth(thickness);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);

            if (type == "rectangle") {
                int ax = qRound(ann["x"].toDouble() * annScale);
                int ay = qRound(ann["y"].toDouble() * annScale);
                int aw = qRound(ann["w"].toDouble() * annScale);
                int ah = qRound(ann["h"].toDouble() * annScale);
                painter.drawRect(ax, ay, aw, ah);
            } else if (type == "ellipse") {
                int ax = qRound(ann["x"].toDouble() * annScale);
                int ay = qRound(ann["y"].toDouble() * annScale);
                int aw = qRound(ann["w"].toDouble() * annScale);
                int ah = qRound(ann["h"].toDouble() * annScale);
                painter.drawEllipse(ax, ay, aw, ah);
            } else if (type == "arrow") {
                int x1 = qRound(ann["x1"].toDouble() * annScale);
                int y1 = qRound(ann["y1"].toDouble() * annScale);
                int x2 = qRound(ann["x2"].toDouble() * annScale);
                int y2 = qRound(ann["y2"].toDouble() * annScale);

                // Draw line
                painter.drawLine(x1, y1, x2, y2);

                // Draw arrowhead
                double angle = std::atan2(y2 - y1, x2 - x1);
                int arrowSize = qRound((10 + thickness) * annScale);
                QPolygon arrowHead;
                arrowHead << QPoint(x2, y2)
                          << QPoint(x2 - arrowSize * std::cos(angle - M_PI/6),
                                   y2 - arrowSize * std::sin(angle - M_PI/6))
                          << QPoint(x2 - arrowSize * std::cos(angle + M_PI/6),
                                   y2 - arrowSize * std::sin(angle + M_PI/6));
                painter.setBrush(annColor);
                painter.drawPolygon(arrowHead);
                painter.setBrush(Qt::NoBrush);
            } else if (type == "freehand") {
                QVariantList points = ann["points"].toList();
                if (points.size() > 1) {
                    QVariantMap p0 = points[0].toMap();
                    double startX = p0["x"].toDouble() * annScale;
                    double startY = p0["y"].toDouble() * annScale;

                    QPainterPath path;
                    path.moveTo(startX, startY);
                    for (int i = 1; i < points.size(); ++i) {
                        QVariantMap p = points[i].toMap();
                        path.lineTo(p["x"].toDouble() * annScale, p["y"].toDouble() * annScale);
                    }
                    painter.drawPath(path);
                }
            } else if (type == "text") {
                int ax = qRound(ann["x"].toDouble() * annScale);
                int ay = qRound(ann["y"].toDouble() * annScale);
                QString text = ann["text"].toString();
                int fontSize = qRound(ann["fontSize"].toInt() * annScale);

                QFont font("Arial", fontSize);
                painter.setFont(font);
                painter.setPen(annColor);
                painter.drawText(ax, ay, text);
            }
        }

        painter.end();
    }

    // Add timestamp watermark to bottom-right corner (scale font)
    {
        QPainter painter(&cropped);
        painter.setRenderHint(QPainter::Antialiasing);

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        int fontSize = qRound(14 * scale);
        QFont font("Arial", fontSize);
        font.setBold(true);
        painter.setFont(font);

        QFontMetrics fm(font);
        int textWidth = fm.horizontalAdvance(timestamp);
        int margin = qRound(10 * scale);

        int textX = cropped.width() - textWidth - margin;
        int textY = cropped.height() - margin;

        // Draw shadow for better visibility
        painter.setPen(QColor(0, 0, 0, 180));
        painter.drawText(textX + 1, textY + 1, timestamp);

        // Draw white text
        painter.setPen(QColor(255, 255, 255, 230));
        painter.drawText(textX, textY, timestamp);

        painter.end();
    }

    // Generate filename and save
    QString filename = generateFilename(m_imageFormat);
    QString filePath = m_outputDirectory + "/" + filename;

    if (cropped.save(filePath)) {
        UIVIEW_LOG_DEBUG("Screenshot saved to: " << filePath.toStdString());
        result["success"] = true;
        result["filePath"] = filePath;

        // Clear current screenshot
        m_currentScreenshot = QImage();

        emit captureCompleted(filePath);
    } else {
        result["success"] = false;
        result["error"] = tr("Failed to save screenshot");
    }

    return result;
}
