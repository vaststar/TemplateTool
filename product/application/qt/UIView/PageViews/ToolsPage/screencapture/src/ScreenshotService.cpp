#include "PageViews/ToolsPage/screencapture/include/ScreenshotService.h"

#include <QGuiApplication>
#include <QScreen>
#include <QPainter>
#include <QDateTime>

#ifdef Q_OS_MAC
#include <ApplicationServices/ApplicationServices.h>
#include <dlfcn.h>

// Typedefs for dynamically loaded functions (deprecated in macOS 15)
typedef CFArrayRef (*CGWindowListCopyWindowInfoFunc)(CGWindowListOption, CGWindowID);
typedef CGImageRef (*CGWindowListCreateImageFunc)(CGRect, CGWindowListOption, CGWindowID, CGWindowImageOption);

static CGWindowListCopyWindowInfoFunc getCGWindowListCopyWindowInfo() {
    static CGWindowListCopyWindowInfoFunc func = nullptr;
    static bool loaded = false;
    if (!loaded) {
        void* handle = dlopen("/System/Library/Frameworks/CoreGraphics.framework/CoreGraphics", RTLD_LAZY);
        if (handle) {
            func = (CGWindowListCopyWindowInfoFunc)dlsym(handle, "CGWindowListCopyWindowInfo");
        }
        loaded = true;
    }
    return func;
}

static CGWindowListCreateImageFunc getCGWindowListCreateImage() {
    static CGWindowListCreateImageFunc func = nullptr;
    static bool loaded = false;
    if (!loaded) {
        void* handle = dlopen("/System/Library/Frameworks/CoreGraphics.framework/CoreGraphics", RTLD_LAZY);
        if (handle) {
            func = (CGWindowListCreateImageFunc)dlsym(handle, "CGWindowListCreateImage");
        }
        loaded = true;
    }
    return func;
}
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#endif

// === Cross-platform (Qt native) ===

QImage ScreenshotService::captureFullScreen()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return QImage();
    }

    QPixmap pixmap = screen->grabWindow(0);
    return pixmap.toImage();
}

QImage ScreenshotService::captureRegion(const QRect& region)
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return QImage();
    }

    QPixmap pixmap = screen->grabWindow(0, region.x(), region.y(),
                                         region.width(), region.height());
    return pixmap.toImage();
}

QImage ScreenshotService::captureScreen(int screenIndex)
{
    QList<QScreen*> screens = QGuiApplication::screens();
    if (screenIndex < 0 || screenIndex >= screens.size()) {
        return QImage();
    }

    QScreen* screen = screens[screenIndex];
    QPixmap pixmap = screen->grabWindow(0);
    return pixmap.toImage();
}

int ScreenshotService::screenCount()
{
    return QGuiApplication::screens().size();
}

QImage ScreenshotService::addTimestamp(const QImage& image, const QString& format)
{
    QImage result = image.copy();
    QPainter painter(&result);

    QString timestamp = QDateTime::currentDateTime().toString(format);

    // Calculate position and font
    QFont font("Arial", 12);
    painter.setFont(font);
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(timestamp);

    int padding = 8;
    int x = result.width() - textRect.width() - padding - 10;
    int y = result.height() - padding - 5;

    // Draw background
    QRect bgRect(x - padding, y - textRect.height() - padding/2,
                 textRect.width() + padding * 2, textRect.height() + padding);
    painter.fillRect(bgRect, QColor(0, 0, 0, 128));

    // Draw text
    painter.setPen(Qt::white);
    painter.drawText(x, y, timestamp);

    return result;
}

// === Platform-specific: Window List ===

QVariantList ScreenshotService::getWindowListAsVariant()
{
    QVariantList result;
    QList<WindowInfo> windows = getWindowList();

    for (const WindowInfo& w : windows) {
        QVariantMap map;
        map["windowId"] = w.windowId;
        map["name"] = w.name;
        map["ownerName"] = w.ownerName;
        map["x"] = w.bounds.x();
        map["y"] = w.bounds.y();
        map["width"] = w.bounds.width();
        map["height"] = w.bounds.height();
        map["isOnScreen"] = w.isOnScreen;
        result.append(map);
    }

    return result;
}

QList<WindowInfo> ScreenshotService::getWindowList()
{
#ifdef Q_OS_MAC
    return getWindowListMac();
#elif defined(Q_OS_WIN)
    return getWindowListWin();
#elif defined(Q_OS_LINUX)
    return getWindowListLinux();
#else
    return QList<WindowInfo>();
#endif
}

QImage ScreenshotService::captureWindow(qint64 windowId)
{
#ifdef Q_OS_MAC
    return captureWindowMac(windowId);
#elif defined(Q_OS_WIN)
    return captureWindowWin(windowId);
#elif defined(Q_OS_LINUX)
    return captureWindowLinux(windowId);
#else
    Q_UNUSED(windowId)
    return QImage();
#endif
}

// === macOS Implementation ===

#ifdef Q_OS_MAC

QList<WindowInfo> ScreenshotService::getWindowListMac()
{
    QList<WindowInfo> result;

    // Use dynamically loaded function (deprecated in macOS 15)
    auto func = getCGWindowListCopyWindowInfo();
    if (!func) {
        return result;
    }

    CFArrayRef windowList = func(
        kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
        kCGNullWindowID
    );

    if (!windowList) {
        return result;
    }

    CFIndex count = CFArrayGetCount(windowList);
    for (CFIndex i = 0; i < count; ++i) {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);

        // Get window ID
        CGWindowID windowId = 0;
        CFNumberRef windowIdRef = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowNumber);
        if (windowIdRef) {
            CFNumberGetValue(windowIdRef, kCFNumberIntType, &windowId);
        }

        // Get window name
        QString name;
        CFStringRef nameRef = (CFStringRef)CFDictionaryGetValue(windowInfo, kCGWindowName);
        if (nameRef) {
            char buffer[256];
            if (CFStringGetCString(nameRef, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
                name = QString::fromUtf8(buffer);
            }
        }

        // Get owner (app) name
        QString ownerName;
        CFStringRef ownerRef = (CFStringRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerName);
        if (ownerRef) {
            char buffer[256];
            if (CFStringGetCString(ownerRef, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
                ownerName = QString::fromUtf8(buffer);
            }
        }

        // Get bounds
        CGRect bounds = CGRectZero;
        CFDictionaryRef boundsRef = (CFDictionaryRef)CFDictionaryGetValue(windowInfo, kCGWindowBounds);
        if (boundsRef) {
            CGRectMakeWithDictionaryRepresentation(boundsRef, &bounds);
        }

        // Skip windows without name or too small
        if (bounds.size.width < 50 || bounds.size.height < 50) {
            continue;
        }

        // Skip certain system windows
        if (ownerName == "Window Server" || ownerName == "Dock") {
            continue;
        }

        WindowInfo info;
        info.windowId = windowId;
        info.name = name.isEmpty() ? ownerName : name;
        info.ownerName = ownerName;
        info.bounds = QRect(bounds.origin.x, bounds.origin.y,
                           bounds.size.width, bounds.size.height);
        info.isOnScreen = true;

        result.append(info);
    }

    CFRelease(windowList);
    return result;
}

QImage ScreenshotService::captureWindowMac(qint64 windowId)
{
    // Use dynamically loaded function (deprecated in macOS 15)
    auto func = getCGWindowListCreateImage();
    if (!func) {
        return QImage();
    }

    CGImageRef cgImage = func(
        CGRectNull,
        kCGWindowListOptionIncludingWindow,
        static_cast<CGWindowID>(windowId),
        kCGWindowImageBoundsIgnoreFraming
    );

    if (!cgImage) {
        return QImage();
    }

    size_t width = CGImageGetWidth(cgImage);
    size_t height = CGImageGetHeight(cgImage);

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(
        image.bits(),
        width, height,
        8,
        image.bytesPerLine(),
        colorSpace,
        kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host
    );

    CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);

    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    CGImageRelease(cgImage);

    return image;
}

#endif // Q_OS_MAC

// === Windows Implementation ===

#ifdef Q_OS_WIN

struct EnumWindowsData {
    QList<WindowInfo>* windows;
};

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    EnumWindowsData* data = reinterpret_cast<EnumWindowsData*>(lParam);

    if (!IsWindowVisible(hwnd)) {
        return TRUE;
    }

    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        return TRUE;
    }

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    if (width < 50 || height < 50) {
        return TRUE;
    }

    wchar_t title[256];
    GetWindowTextW(hwnd, title, 256);
    QString name = QString::fromWCharArray(title);

    if (name.isEmpty()) {
        return TRUE;
    }

    WindowInfo info;
    info.windowId = reinterpret_cast<qint64>(hwnd);
    info.name = name;
    info.ownerName = name;  // Could get process name here
    info.bounds = QRect(rect.left, rect.top, width, height);
    info.isOnScreen = true;

    data->windows->append(info);

    return TRUE;
}

QList<WindowInfo> ScreenshotService::getWindowListWin()
{
    QList<WindowInfo> result;
    EnumWindowsData data = { &result };
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));
    return result;
}

QImage ScreenshotService::captureWindowWin(qint64 windowId)
{
    HWND hwnd = reinterpret_cast<HWND>(windowId);

    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        return QImage();
    }

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);
    SelectObject(hdcMem, hBitmap);

    // Try PrintWindow first (works better with some windows)
    if (!PrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT)) {
        BitBlt(hdcMem, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);
    }

    QImage image(width, height, QImage::Format_ARGB32);

    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  // Negative for top-down
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    GetDIBits(hdcMem, hBitmap, 0, height, image.bits(),
              reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);

    return image;
}

#endif // Q_OS_WIN

// === Linux Implementation (stub) ===

#ifdef Q_OS_LINUX

QList<WindowInfo> ScreenshotService::getWindowListLinux()
{
    // TODO: Implement using X11/XCB or PipeWire for Wayland
    return QList<WindowInfo>();
}

QImage ScreenshotService::captureWindowLinux(qint64 windowId)
{
    // TODO: Implement using X11/XCB
    Q_UNUSED(windowId)
    return QImage();
}

#endif // Q_OS_LINUX
