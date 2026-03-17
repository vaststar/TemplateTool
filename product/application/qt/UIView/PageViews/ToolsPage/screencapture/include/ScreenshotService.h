#pragma once

#include <QImage>
#include <QRect>
#include <QVariantList>
#include <QString>

/**
 * @brief Window information for window capture
 */
struct WindowInfo
{
    qint64 windowId;
    QString name;
    QString ownerName;
    QRect bounds;
    bool isOnScreen;
};

/**
 * @brief Screenshot Service - handles platform-specific screen capture
 *
 * Provides cross-platform screenshot functionality:
 * - Full screen capture (Qt native)
 * - Region capture (Qt native)
 * - Window capture (platform-specific)
 * - Window enumeration (platform-specific)
 */
class ScreenshotService
{
public:
    ScreenshotService() = default;
    ~ScreenshotService() = default;

    // === Cross-platform (Qt native) ===

    /**
     * @brief Capture the entire screen
     * @return QImage of the full screen
     */
    static QImage captureFullScreen();

    /**
     * @brief Capture a specific region of the screen
     * @param region The region to capture
     * @return QImage of the specified region
     */
    static QImage captureRegion(const QRect& region);

    /**
     * @brief Capture a specific screen by index
     * @param screenIndex Index of the screen (0 = primary)
     * @return QImage of the specified screen
     */
    static QImage captureScreen(int screenIndex = 0);

    // === Platform-specific ===

    /**
     * @brief Get list of all visible windows
     * @return List of WindowInfo structures
     */
    static QList<WindowInfo> getWindowList();

    /**
     * @brief Convert window list to QVariantList for QML
     * @return QVariantList suitable for QML ListView
     */
    static QVariantList getWindowListAsVariant();

    /**
     * @brief Capture a specific window by ID
     * @param windowId Platform-specific window identifier
     * @return QImage of the window content
     */
    static QImage captureWindow(qint64 windowId);

    // === Utility ===

    /**
     * @brief Add timestamp watermark to image
     * @param image The image to modify
     * @param format Timestamp format (default: "yyyy-MM-dd hh:mm:ss")
     * @return QImage with timestamp added
     */
    static QImage addTimestamp(const QImage& image, const QString& format = "yyyy-MM-dd hh:mm:ss");

    /**
     * @brief Get the number of available screens
     * @return Number of screens
     */
    static int screenCount();

private:
#ifdef Q_OS_MAC
    static QImage captureWindowMac(qint64 windowId);
    static QList<WindowInfo> getWindowListMac();
#endif

#ifdef Q_OS_WIN
    static QImage captureWindowWin(qint64 windowId);
    static QList<WindowInfo> getWindowListWin();
#endif

#ifdef Q_OS_LINUX
    static QImage captureWindowLinux(qint64 windowId);
    static QList<WindowInfo> getWindowListLinux();
#endif
};
