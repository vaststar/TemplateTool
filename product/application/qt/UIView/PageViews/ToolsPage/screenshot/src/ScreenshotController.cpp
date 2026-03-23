#include "PageViews/ToolsPage/screenshot/include/ScreenshotController.h"
#include "LoggerDefine/LoggerDefine.h"
#include "UIEvents/UIMainWindowEvent.h"

#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <commonHead/viewModels/ScreenshotViewModel/IScreenshotViewModel.h>
#include <commonHead/viewModels/ScreenshotViewModel/IScreenshotModel.h>

#include <QGuiApplication>
#include <QClipboard>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QImage>
#include <QWindow>
#include <QThread>
#include <QTimer>

using namespace commonHead::viewModels;
using namespace commonHead::viewModels::model;

// ============================================================================
// Helper: parse "#RRGGBB" or "#AARRGGBB" color string → r,g,b,a
// ============================================================================
static void parseColor(const QString& colorStr, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
{
    QColor c(colorStr);
    if (!c.isValid()) { r = 255; g = 0; b = 0; a = 255; return; }
    r = static_cast<uint8_t>(c.red());
    g = static_cast<uint8_t>(c.green());
    b = static_cast<uint8_t>(c.blue());
    a = static_cast<uint8_t>(c.alpha());
}

// ============================================================================
// Construction
// ============================================================================

ScreenshotController::ScreenshotController(QObject* parent)
    : UIViewController(parent)
    , m_viewModelEmitter(std::make_shared<UIVMSignalEmitter::ScreenshotViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create ScreenshotController");
}

ScreenshotController::~ScreenshotController() = default;

// ============================================================================
// init — called after AppContext injection
// ============================================================================

void ScreenshotController::init()
{
    UIVIEW_LOG_DEBUG("ScreenshotController::init");

    // Phase 1: Connect emitter Qt signals → controller slots
    using Emitter = UIVMSignalEmitter::ScreenshotViewModelEmitter;
    connect(m_viewModelEmitter.get(), &Emitter::signals_onStateChanged,
            this, &ScreenshotController::onVMStateChanged);
    connect(m_viewModelEmitter.get(), &Emitter::signals_onScreenCaptured,
            this, &ScreenshotController::onVMScreenCaptured);
    connect(m_viewModelEmitter.get(), &Emitter::signals_onAnnotationsChanged,
            this, &ScreenshotController::onVMAnnotationsChanged);
    connect(m_viewModelEmitter.get(), &Emitter::signals_onScreenshotSaved,
            this, &ScreenshotController::onVMScreenshotSaved);
    connect(m_viewModelEmitter.get(), &Emitter::signals_onSettingsChanged,
            this, &ScreenshotController::onVMSettingsChanged);
    connect(m_viewModelEmitter.get(), &Emitter::signals_onError,
            this, &ScreenshotController::onVMError);

    // Phase 2: Create ViewModel from factory
    m_viewModel = getAppContext()->getViewModelFactory()->createScreenshotViewModelInstance();
    m_viewModel->initViewModel();

    // Phase 3: Register emitter as callback
    m_viewModel->registerCallback(m_viewModelEmitter);

    // Sync initial settings from ViewModel
    auto settings = m_viewModel->getSettings();
    m_outputDirectory = QString::fromStdString(settings.outputDirectory);
    m_imageFormat = QString::fromStdString(settings.imageFormat);
    m_delaySeconds = settings.captureDelay;
    m_includeTimestamp = settings.addTimestamp;

    // If outputDirectory is not set, use a sensible default
    if (m_outputDirectory.isEmpty()) {
        m_outputDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                            + "/Screenshots";
        QDir().mkpath(m_outputDirectory);

        // Push default back to ViewModel
        ScreenshotSettings s = settings;
        s.outputDirectory = m_outputDirectory.toStdString();
        m_viewModel->updateSettings(s);
    }
}

// ============================================================================
// Property Getters
// ============================================================================

bool ScreenshotController::hasScreenshot() const { return m_hasScreenshot; }
QString ScreenshotController::screenshotBase64() const { return m_screenshotBase64; }
int ScreenshotController::screenshotWidth() const { return m_screenshotWidth; }
int ScreenshotController::screenshotHeight() const { return m_screenshotHeight; }
int ScreenshotController::screenshotState() const { return m_state; }

QString ScreenshotController::outputDirectory() const { return m_outputDirectory; }

QString ScreenshotController::screenshotsFolderUrl() const
{
    return QStringLiteral("file://") + m_outputDirectory;
}

int ScreenshotController::delaySeconds() const { return m_delaySeconds; }
QString ScreenshotController::imageFormat() const { return m_imageFormat; }
bool ScreenshotController::includeTimestamp() const { return m_includeTimestamp; }

// ============================================================================
// Property Setters — push to ViewModel
// ============================================================================

void ScreenshotController::setOutputDirectory(const QString& path)
{
    if (m_outputDirectory != path) {
        m_outputDirectory = path;
        if (m_viewModel) {
            auto s = m_viewModel->getSettings();
            s.outputDirectory = path.toStdString();
            m_viewModel->updateSettings(s);
        }
    }
}

void ScreenshotController::setDelaySeconds(int seconds)
{
    if (m_delaySeconds != seconds) {
        m_delaySeconds = seconds;
        if (m_viewModel) {
            auto s = m_viewModel->getSettings();
            s.captureDelay = seconds;
            m_viewModel->updateSettings(s);
        }
    }
}

void ScreenshotController::setImageFormat(const QString& format)
{
    if (m_imageFormat != format) {
        m_imageFormat = format;
        if (m_viewModel) {
            auto s = m_viewModel->getSettings();
            s.imageFormat = format.toStdString();
            m_viewModel->updateSettings(s);
        }
    }
}

void ScreenshotController::setIncludeTimestamp(bool include)
{
    if (m_includeTimestamp != include) {
        m_includeTimestamp = include;
        if (m_viewModel) {
            auto s = m_viewModel->getSettings();
            s.addTimestamp = include;
            m_viewModel->updateSettings(s);
        }
    }
}

// ============================================================================

// Capture Methods — delegate to ViewModel
// ============================================================================

void ScreenshotController::captureFullScreen()
{
    if (!m_viewModel) return;
    UIVIEW_LOG_DEBUG("captureFullScreen -> hide window, then capture");

    // Hide the main window via EventBus so it doesn't appear in the screenshot
    sendUIEvent<UIMainWindowEvent>(UIMainWindowEvent::Action::Hide);

    // Wait for the window to finish hiding, then capture
    QTimer::singleShot(300, this, [this]() {
        if (!m_viewModel) return;

        m_viewModel->captureFullScreen();
        m_viewModel->saveScreenshot();

        // Restore the main window
        sendUIEvent<UIMainWindowEvent>(UIMainWindowEvent::Action::Show);
    });
}

void ScreenshotController::captureWindow(qint64 windowId)
{
    if (!m_viewModel) return;
    UIVIEW_LOG_DEBUG("captureWindow: " << windowId << " -> ViewModel");
    m_viewModel->captureWindow(static_cast<int64_t>(windowId));
    // Immediately save to file
    m_viewModel->saveScreenshot();
}

QVariantList ScreenshotController::getWindowList()
{
    QVariantList result;
    if (!m_viewModel) return result;
    auto windows = m_viewModel->getWindowList();

    // Get our own process name to filter it out
    QString ownAppName = QGuiApplication::applicationDisplayName();
    if (ownAppName.isEmpty()) {
        ownAppName = QGuiApplication::applicationName();
    }

    for (const auto& w : windows) {
        // Skip windows with empty names
        QString name = QString::fromStdString(w.name);
        QString owner = QString::fromStdString(w.ownerName);
        if (name.isEmpty() && owner.isEmpty()) continue;

        // Skip our own app windows
        if (!ownAppName.isEmpty() && owner == ownAppName) continue;

        QVariantMap map;
        map["windowId"] = static_cast<qint64>(w.windowId);
        map["name"] = name.isEmpty() ? owner : name;
        map["ownerName"] = owner;
        result.append(map);
    }
    return result;
}

// ============================================================================
// Annotation Methods — build AnnotationData and delegate
// ============================================================================

void ScreenshotController::addRectangle(int x, int y, int w, int h,
                                         const QString& color, int thickness)
{
    AnnotationData ann;
    ann.type = "rectangle";
    ann.x = x; ann.y = y; ann.w = w; ann.h = h;
    parseColor(color, ann.r, ann.g, ann.b, ann.a);
    ann.thickness = thickness;
    if (m_viewModel) m_viewModel->addAnnotation(ann);
}

void ScreenshotController::addEllipse(int x, int y, int w, int h,
                                       const QString& color, int thickness)
{
    AnnotationData ann;
    ann.type = "ellipse";
    ann.x = x; ann.y = y; ann.w = w; ann.h = h;
    parseColor(color, ann.r, ann.g, ann.b, ann.a);
    ann.thickness = thickness;
    if (m_viewModel) m_viewModel->addAnnotation(ann);
}

void ScreenshotController::addArrow(int x1, int y1, int x2, int y2,
                                     const QString& color, int thickness)
{
    AnnotationData ann;
    ann.type = "arrow";
    ann.startX = x1; ann.startY = y1;
    ann.endX = x2; ann.endY = y2;
    parseColor(color, ann.r, ann.g, ann.b, ann.a);
    ann.thickness = thickness;
    if (m_viewModel) m_viewModel->addAnnotation(ann);
}

void ScreenshotController::addText(int x, int y, const QString& text,
                                    const QString& color, int fontSize)
{
    AnnotationData ann;
    ann.type = "text";
    ann.x = x; ann.y = y;
    ann.text = text.toStdString();
    parseColor(color, ann.r, ann.g, ann.b, ann.a);
    ann.fontSize = fontSize;
    if (m_viewModel) m_viewModel->addAnnotation(ann);
}

void ScreenshotController::addFreehand(const QVariantList& points,
                                        const QString& color, int thickness)
{
    AnnotationData ann;
    ann.type = "freehand";
    parseColor(color, ann.r, ann.g, ann.b, ann.a);
    ann.thickness = thickness;

    ann.points.reserve(points.size());
    for (const QVariant& pt : points) {
        QVariantMap map = pt.toMap();
        ann.points.emplace_back(map["x"].toDouble(), map["y"].toDouble());
    }
    if (m_viewModel) m_viewModel->addAnnotation(ann);
}

void ScreenshotController::addMosaic(int x, int y, int w, int h, int blockSize)
{
    AnnotationData ann;
    ann.type = "mosaic";
    ann.x = x; ann.y = y; ann.w = w; ann.h = h;
    ann.mosaicBlockSize = blockSize;
    if (m_viewModel) m_viewModel->addAnnotation(ann);
}

void ScreenshotController::undo()       { if (m_viewModel) m_viewModel->undo(); }
void ScreenshotController::redo()       { if (m_viewModel) m_viewModel->redo(); }
void ScreenshotController::clearAnnotations() { if (m_viewModel) m_viewModel->clearAnnotations(); }

// ============================================================================
// Save / Export — delegate to ViewModel
// ============================================================================

void ScreenshotController::saveScreenshot()
{
    if (!m_viewModel) return;
    UIVIEW_LOG_DEBUG("saveScreenshot -> ViewModel");
    m_viewModel->saveScreenshot();
}

void ScreenshotController::discardScreenshot()
{
    if (!m_viewModel) return;
    UIVIEW_LOG_DEBUG("discardScreenshot -> ViewModel");
    m_viewModel->discardCapture();
}

QString ScreenshotController::getDefaultSavePath()
{
    if (!m_outputDirectory.isEmpty() && QDir(m_outputDirectory).exists()) {
        return m_outputDirectory;
    }
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

// ============================================================================
// Overlay Methods — delegate to ViewModel
// ============================================================================

QVariantMap ScreenshotController::grabScreenForOverlay()
{
    QVariantMap result;
    if (!m_viewModel) {
        result["success"] = false;
        result["error"] = QStringLiteral("ViewModel not initialized");
        return result;
    }

    UIVIEW_LOG_DEBUG("grabScreenForOverlay -> ViewModel");

    // Hide the app window via EventBus so it doesn't appear in the screenshot
    sendUIEvent<UIMainWindowEvent>(UIMainWindowEvent::Action::Hide);

    // Process events to let the hide take effect, then wait briefly
    QGuiApplication::processEvents();
    QThread::msleep(300);
    QGuiApplication::processEvents();

    // captureFullScreen is synchronous — it captures, converts to base64,
    // and fires onScreenCaptured callback which updates m_screenshotBase64/Width/Height
    m_viewModel->captureFullScreen();

    // Restore the app window via EventBus
    sendUIEvent<UIMainWindowEvent>(UIMainWindowEvent::Action::Show);

    // After captureFullScreen returns, the callback has already run (same thread),
    // so m_screenshotBase64/Width/Height are up-to-date.
    if (m_screenshotBase64.isEmpty()) {
        result["success"] = false;
        result["error"] = QStringLiteral("Screen capture failed");
        return result;
    }

    result["success"] = true;
    result["base64"] = m_screenshotBase64;
    result["width"] = m_screenshotWidth;
    result["height"] = m_screenshotHeight;

    UIVIEW_LOG_DEBUG("grabScreenForOverlay captured: " << m_screenshotWidth << "x" << m_screenshotHeight);
    return result;
}

QVariantMap ScreenshotController::saveRegionScreenshot(int x, int y, int w, int h,
                                                       int paintedWidth, int paintedHeight,
                                                       const QVariantList& annotations)
{
    QVariantMap result;
    UIVIEW_LOG_DEBUG("saveRegionScreenshot -> ViewModel");
    if (!m_viewModel) {
        result["success"] = false;
        result["error"] = "ViewModel not initialized";
        return result;
    }

    // Compute scale factor from painted size vs actual captured image size (floating point for precision)
    double scaleFactor = 1.0;
    if (paintedWidth > 0) {
        scaleFactor = static_cast<double>(m_screenshotWidth) / static_cast<double>(paintedWidth);
        if (scaleFactor < 1.0) scaleFactor = 1.0;
    }

    // Add annotations from QML overlay first
    m_viewModel->clearAnnotations();
    for (const QVariant& annVar : annotations) {
        QVariantMap map = annVar.toMap();
        AnnotationData ann;
        ann.type = map["type"].toString().toStdString();

        uint8_t r, g, b, a;
        parseColor(map["color"].toString(), r, g, b, a);
        ann.r = r; ann.g = g; ann.b = b; ann.a = a;
        ann.thickness = map["thickness"].toInt();

        if (ann.type == "rectangle" || ann.type == "ellipse" || ann.type == "mosaic") {
            ann.x = map["x"].toInt();
            ann.y = map["y"].toInt();
            ann.w = map["w"].toInt();
            ann.h = map["h"].toInt();
            if (ann.type == "mosaic") {
                ann.mosaicBlockSize = map.value("blockSize", 10).toInt();
            }
        } else if (ann.type == "arrow" || ann.type == "line") {
            ann.startX = map["x1"].toInt();
            ann.startY = map["y1"].toInt();
            ann.endX = map["x2"].toInt();
            ann.endY = map["y2"].toInt();
        } else if (ann.type == "text") {
            ann.x = map["x"].toInt();
            ann.y = map["y"].toInt();
            ann.text = map["text"].toString().toStdString();
            ann.fontSize = map.value("fontSize", 16).toInt();
        } else if (ann.type == "freehand") {
            QVariantList pts = map["points"].toList();
            ann.points.reserve(pts.size());
            for (const QVariant& pt : pts) {
                QVariantMap pm = pt.toMap();
                ann.points.emplace_back(pm["x"].toDouble(), pm["y"].toDouble());
            }
        }

        m_viewModel->addAnnotation(ann);
    }

    // Delegate region crop + save
    m_viewModel->selectRegionAndSave(x, y, w, h, scaleFactor);

    result["success"] = true;
    return result;
}

// ============================================================================
// Permission
// ============================================================================

bool ScreenshotController::hasPermission()
{
    return m_viewModel ? m_viewModel->hasPermission() : false;
}

void ScreenshotController::requestPermission()
{
    if (m_viewModel) m_viewModel->requestPermission();
}

// ============================================================================
// File Browser Methods — remain in UI layer (platform Qt APIs)
// ============================================================================

void ScreenshotController::openScreenshotsFolder()
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

void ScreenshotController::openFile(const QString& filePath)
{
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

void ScreenshotController::copyFileToClipboard(const QString& filePath)
{
    QImage image(filePath);
    if (image.isNull()) {
        emit errorOccurred(tr("Failed to load image: %1").arg(filePath));
        return;
    }
    QGuiApplication::clipboard()->setImage(image);
}

void ScreenshotController::revealInFinder(const QString& filePath)
{
    if (!QFile::exists(filePath)) {
        emit errorOccurred(tr("File not found: %1").arg(filePath));
        return;
    }
#ifdef Q_OS_MAC
    QProcess::startDetached("open", {"-R", filePath});
#elif defined(Q_OS_WIN)
    QProcess::startDetached("explorer", {"/select,", QDir::toNativeSeparators(filePath)});
#elif defined(Q_OS_LINUX)
    QFileInfo fi(filePath);
    QProcess::startDetached("xdg-open", {fi.absolutePath()});
#endif
}

void ScreenshotController::deleteFile(const QString& filePath)
{
    if (!QFile::exists(filePath)) {
        emit errorOccurred(tr("File not found: %1").arg(filePath));
        return;
    }
    if (!QFile::remove(filePath)) {
        emit errorOccurred(tr("Failed to delete file: %1").arg(filePath));
    }
}

// ============================================================================
// ViewModel Callback Slots
// ============================================================================

void ScreenshotController::onVMStateChanged(int state)
{
    UIVIEW_LOG_DEBUG("onVMStateChanged: " << state);
    if (m_state != state) {
        m_state = state;
        m_hasScreenshot = (state != static_cast<int>(ScreenshotState::Idle));
        emit stateChanged();
        emit screenshotChanged();
    }
}

void ScreenshotController::onVMScreenCaptured(const QString& base64Png, int width, int height)
{
    UIVIEW_LOG_DEBUG("onVMScreenCaptured: " << width << "x" << height);
    m_screenshotBase64 = base64Png;
    m_screenshotWidth = width;
    m_screenshotHeight = height;
    m_hasScreenshot = true;
    emit screenshotChanged();
}

void ScreenshotController::onVMAnnotationsChanged()
{
    emit annotationsChanged();
}

void ScreenshotController::onVMScreenshotSaved(const QString& filePath)
{
    UIVIEW_LOG_DEBUG("onVMScreenshotSaved: " << filePath.toStdString());
    emit captureCompleted(filePath);
}

void ScreenshotController::onVMSettingsChanged(const ScreenshotSettings& settings)
{
    bool changed = false;
    QString newDir = QString::fromStdString(settings.outputDirectory);
    QString newFmt = QString::fromStdString(settings.imageFormat);

    if (m_outputDirectory != newDir) { m_outputDirectory = newDir; changed = true; }
    if (m_imageFormat != newFmt) { m_imageFormat = newFmt; changed = true; }
    if (m_delaySeconds != settings.captureDelay) { m_delaySeconds = settings.captureDelay; changed = true; }
    if (m_includeTimestamp != settings.addTimestamp) { m_includeTimestamp = settings.addTimestamp; changed = true; }

    if (changed) {
        emit settingsChanged();
    }
}

void ScreenshotController::onVMError(const QString& message)
{
    UIVIEW_LOG_DEBUG("onVMError: " << message.toStdString());
    emit errorOccurred(message);
}
