#include "ScreenCaptureUtils_Mac.h"

#ifdef __APPLE__

#import <ScreenCaptureKit/ScreenCaptureKit.h>
#import <CoreGraphics/CoreGraphics.h>
#include <dispatch/dispatch.h>
#include <algorithm>
#include <cstring>

namespace ucf::utilities::screencapture {

// ============================================================================
// Internal helpers
// ============================================================================

/**
 * @brief Convert a CGImage to CaptureImage (BGRA pixel data)
 */
static CaptureImage cgImageToCaptureImage(CGImageRef cgImage, int scaleFactor = 1)
{
    if (!cgImage) {
        return {};
    }

    int width = static_cast<int>(CGImageGetWidth(cgImage));
    int height = static_cast<int>(CGImageGetHeight(cgImage));
    int bytesPerRow = width * 4;

    CaptureImage result;
    result.width = width;
    result.height = height;
    result.bytesPerRow = bytesPerRow;
    result.scaleFactor = scaleFactor;
    result.pixels.resize(static_cast<size_t>(bytesPerRow) * height);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(
        result.pixels.data(), width, height, 8, bytesPerRow,
        colorSpace,
        static_cast<CGBitmapInfo>(kCGImageAlphaPremultipliedFirst) | kCGBitmapByteOrder32Little); // BGRA

    if (ctx) {
        CGContextDrawImage(ctx, CGRectMake(0, 0, width, height), cgImage);
        CGContextRelease(ctx);
    }
    CGColorSpaceRelease(colorSpace);

    return result;
}

/**
 * @brief Synchronously fetch SCShareableContent (bridges async Obj-C API).
 *
 * NOTE: This project does NOT use ARC. Returned object is retained;
 *       the caller MUST call [result release] when done.
 */
static SCShareableContent* getShareableContentSync(bool onScreenOnly = false)
{
    __block SCShareableContent* result = nil;
    dispatch_semaphore_t sem = dispatch_semaphore_create(0);

    [SCShareableContent getShareableContentExcludingDesktopWindows:YES
                                               onScreenWindowsOnly:onScreenOnly
                                                 completionHandler:^(SCShareableContent* content, NSError* err) {
        if (!err && content) {
            result = [content retain];   // MRC: explicitly retain for caller
        }
        dispatch_semaphore_signal(sem);
    }];

    long waitResult = dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, (int64_t)(10 * NSEC_PER_SEC)));
    [sem release];

    if (waitResult != 0) {
        NSLog(@"[ScreenCaptureUtils] getShareableContentSync timed out");
        return nil;
    }
    return result;  // caller owns this reference
}

/**
 * @brief Synchronous screenshot capture via SCScreenshotManager (macOS 14+)
 * @return CGImageRef that the caller must CGImageRelease, or nullptr on failure
 */
static CGImageRef captureWithFilterSync(SCContentFilter* filter, int width, int height)
{
    if (!filter || width <= 0 || height <= 0) return nullptr;

    SCStreamConfiguration* config = [[SCStreamConfiguration alloc] init];
    config.width = width;
    config.height = height;
    config.pixelFormat = kCVPixelFormatType_32BGRA;
    config.showsCursor = NO;

    __block CGImageRef capturedImage = nullptr;
    dispatch_semaphore_t sem = dispatch_semaphore_create(0);

    [SCScreenshotManager captureImageWithFilter:filter
                                  configuration:config
                              completionHandler:^(CGImageRef image, NSError* err) {
        if (image && !err) {
            capturedImage = CGImageRetain(image);
        }
        if (err) {
            NSLog(@"[ScreenCaptureUtils] captureWithFilterSync error: %@", err);
        }
        dispatch_semaphore_signal(sem);
    }];

    long waitResult = dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, (int64_t)(10 * NSEC_PER_SEC)));
    [sem release];
    [config release];

    if (waitResult != 0) {
        NSLog(@"[ScreenCaptureUtils] captureWithFilterSync timed out");
        return nullptr;
    }
    return capturedImage;
}

/**
 * @brief Find SCDisplay matching a CGDirectDisplayID
 */
static SCDisplay* findSCDisplay(NSArray<SCDisplay*>* displays, CGDirectDisplayID displayId)
{
    if (!displays) return nil;
    for (SCDisplay* d in displays) {
        if (d.displayID == displayId) {
            return d;
        }
    }
    return nil;
}

// ============================================================================
// Display Enumeration (CoreGraphics APIs)
// ============================================================================

std::vector<DisplayInfo> ScreenCaptureUtils_Mac::getDisplayList()
{
    std::vector<DisplayInfo> displays;

    CGDirectDisplayID displayIds[16];
    uint32_t displayCount = 0;
    if (CGGetActiveDisplayList(16, displayIds, &displayCount) != kCGErrorSuccess) {
        return displays;
    }

    CGDirectDisplayID mainDisplay = CGMainDisplayID();

    for (uint32_t i = 0; i < displayCount; ++i) {
        DisplayInfo info;
        info.displayId = static_cast<int>(displayIds[i]);

        CGRect bounds = CGDisplayBounds(displayIds[i]);
        info.x = static_cast<int>(bounds.origin.x);
        info.y = static_cast<int>(bounds.origin.y);
        info.width = static_cast<int>(bounds.size.width);
        info.height = static_cast<int>(bounds.size.height);

        size_t pw = CGDisplayPixelsWide(displayIds[i]);
        size_t ph = CGDisplayPixelsHigh(displayIds[i]);
        info.physicalWidth = static_cast<int>(pw);
        info.physicalHeight = static_cast<int>(ph);

        info.scaleFactor = (info.width > 0) ? static_cast<int>(pw / info.width) : 1;
        info.isPrimary = (displayIds[i] == mainDisplay);
        info.name = "Display " + std::to_string(i + 1);

        displays.push_back(std::move(info));
    }

    return displays;
}

int ScreenCaptureUtils_Mac::getDisplayCount()
{
    uint32_t displayCount = 0;
    CGGetActiveDisplayList(0, nullptr, &displayCount);
    return static_cast<int>(displayCount);
}

DisplayInfo ScreenCaptureUtils_Mac::getPrimaryDisplay()
{
    auto displays = getDisplayList();
    for (auto& d : displays) {
        if (d.isPrimary) {
            return d;
        }
    }
    if (!displays.empty()) {
        return displays.front();
    }
    return {};
}

// ============================================================================
// Screen Capture via ScreenCaptureKit (macOS 14+)
// ============================================================================

CaptureImage ScreenCaptureUtils_Mac::captureDisplay(int displayIndex)
{
    @autoreleasepool {
        if (!CGPreflightScreenCaptureAccess()) {
            NSLog(@"[ScreenCaptureUtils] Screen capture permission not granted");
            return {};
        }

        CGDirectDisplayID displayIds[16];
        uint32_t displayCount = 0;
        if (CGGetActiveDisplayList(16, displayIds, &displayCount) != kCGErrorSuccess) {
            return {};
        }

        if (displayIndex < 0 || static_cast<uint32_t>(displayIndex) >= displayCount) {
            return {};
        }

        CGDirectDisplayID targetId = displayIds[displayIndex];
        SCShareableContent* content = getShareableContentSync();  // retained
        if (!content) return {};

        SCDisplay* scDisplay = findSCDisplay(content.displays, targetId);
        if (!scDisplay) {
            [content release];
            return {};
        }

        SCContentFilter* filter = [[SCContentFilter alloc] initWithDisplay:scDisplay
                                                          excludingWindows:@[]];
        if (!filter) {
            [content release];
            return {};
        }

        CGRect bounds = CGDisplayBounds(targetId);
        size_t pw = CGDisplayPixelsWide(targetId);
        size_t ph = CGDisplayPixelsHigh(targetId);
        int scale = (bounds.size.width > 0) ? static_cast<int>(pw / bounds.size.width) : 1;

        CGImageRef cgImage = captureWithFilterSync(filter, static_cast<int>(pw), static_cast<int>(ph));
        [filter release];
        [content release];

        if (!cgImage) return {};

        CaptureImage result = cgImageToCaptureImage(cgImage, scale);
        CGImageRelease(cgImage);
        return result;
    }
}

CaptureImage ScreenCaptureUtils_Mac::captureAllDisplays()
{
    @autoreleasepool {
        if (!CGPreflightScreenCaptureAccess()) {
            NSLog(@"[ScreenCaptureUtils] Screen capture permission not granted");
            return {};
        }

        CGDirectDisplayID displayIds[16];
        uint32_t displayCount = 0;
        if (CGGetActiveDisplayList(16, displayIds, &displayCount) != kCGErrorSuccess || displayCount == 0) {
            return {};
        }

        // Single display — fast path
        if (displayCount == 1) {
            return captureDisplay(0);
        }

        SCShareableContent* content = getShareableContentSync();  // retained
        if (!content) return {};

        // ------------------------------------------------------------------
        // Multi-display: capture each display, then composite into one image
        // ------------------------------------------------------------------

        struct DisplayCapInfo {
            CGDirectDisplayID cgId;
            CGRect bounds;
            int physW, physH;
            int scale;
        };

        std::vector<DisplayCapInfo> caps;
        CGFloat minX = CGFLOAT_MAX, minY = CGFLOAT_MAX;
        CGFloat maxX = -CGFLOAT_MAX, maxY = -CGFLOAT_MAX;

        for (uint32_t i = 0; i < displayCount; ++i) {
            CGRect b = CGDisplayBounds(displayIds[i]);
            size_t pw = CGDisplayPixelsWide(displayIds[i]);
            size_t ph = CGDisplayPixelsHigh(displayIds[i]);
            int scale = (b.size.width > 0) ? static_cast<int>(pw / b.size.width) : 1;

            minX = std::min(minX, b.origin.x);
            minY = std::min(minY, b.origin.y);
            maxX = std::max(maxX, b.origin.x + b.size.width);
            maxY = std::max(maxY, b.origin.y + b.size.height);

            caps.push_back({displayIds[i], b, static_cast<int>(pw), static_cast<int>(ph), scale});
        }

        int maxScale = 1;
        for (auto& c : caps) maxScale = std::max(maxScale, c.scale);

        int totalW = static_cast<int>((maxX - minX) * maxScale);
        int totalH = static_cast<int>((maxY - minY) * maxScale);
        if (totalW <= 0 || totalH <= 0) {
            [content release];
            return {};
        }

        // Create composite BGRA bitmap
        int bytesPerRow = totalW * 4;
        std::vector<uint8_t> compositePixels(static_cast<size_t>(bytesPerRow) * totalH, 0);

        CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
        CGContextRef ctx = CGBitmapContextCreate(
            compositePixels.data(), totalW, totalH, 8, bytesPerRow, cs,
            static_cast<CGBitmapInfo>(kCGImageAlphaPremultipliedFirst) | kCGBitmapByteOrder32Little);
        CGColorSpaceRelease(cs);
        if (!ctx) {
            [content release];
            return {};
        }

        // Capture each display and draw into composite
        for (auto& c : caps) {
            SCDisplay* scDisp = findSCDisplay(content.displays, c.cgId);
            if (!scDisp) continue;

            SCContentFilter* filter = [[SCContentFilter alloc] initWithDisplay:scDisp
                                                              excludingWindows:@[]];
            if (!filter) continue;

            CGImageRef img = captureWithFilterSync(filter, c.physW, c.physH);
            [filter release];
            if (!img) continue;

            CGFloat offsetX = (c.bounds.origin.x - minX) * maxScale;
            CGFloat offsetY = totalH - (c.bounds.origin.y - minY + c.bounds.size.height) * maxScale;
            CGFloat drawW = c.bounds.size.width * maxScale;
            CGFloat drawH = c.bounds.size.height * maxScale;

            CGContextDrawImage(ctx, CGRectMake(offsetX, offsetY, drawW, drawH), img);
            CGImageRelease(img);
        }

        CGContextRelease(ctx);
        [content release];

        CaptureImage result;
        result.width = totalW;
        result.height = totalH;
        result.bytesPerRow = bytesPerRow;
        result.scaleFactor = maxScale;
        result.pixels = std::move(compositePixels);
        return result;
    }
}

// ============================================================================
// Window List / Window Capture via ScreenCaptureKit
// ============================================================================

std::vector<WindowInfo> ScreenCaptureUtils_Mac::getWindowList()
{
    @autoreleasepool {
        std::vector<WindowInfo> windows;

        SCShareableContent* content = getShareableContentSync(/*onScreenOnly=*/true);  // retained
        if (!content) return windows;

        for (SCWindow* w in content.windows) {
            if (w.frame.size.width <= 0 || w.frame.size.height <= 0) continue;
            if (w.windowLayer < 0) continue;

            WindowInfo info;
            info.windowId = w.windowID;
            info.x = static_cast<int>(w.frame.origin.x);
            info.y = static_cast<int>(w.frame.origin.y);
            info.width = static_cast<int>(w.frame.size.width);
            info.height = static_cast<int>(w.frame.size.height);
            info.isOnScreen = true;
            info.isMinimized = false;

            if (w.title) {
                info.name = [w.title UTF8String];
            }
            if (w.owningApplication && w.owningApplication.applicationName) {
                info.ownerName = [w.owningApplication.applicationName UTF8String];
            }

            windows.push_back(std::move(info));
        }

        [content release];
        return windows;
    }
}

CaptureImage ScreenCaptureUtils_Mac::captureWindow(int64_t windowId)
{
    @autoreleasepool {
        if (!CGPreflightScreenCaptureAccess()) {
            return {};
        }

        SCShareableContent* content = getShareableContentSync(/*onScreenOnly=*/false);  // retained
        if (!content) return {};

        SCWindow* targetWindow = nil;
        for (SCWindow* w in content.windows) {
            if (w.windowID == static_cast<CGWindowID>(windowId)) {
                targetWindow = w;
                break;
            }
        }
        if (!targetWindow) {
            [content release];
            return {};
        }

        SCContentFilter* filter = [[SCContentFilter alloc]
            initWithDesktopIndependentWindow:targetWindow];
        if (!filter) {
            [content release];
            return {};
        }

        // Determine scale factor from the display the window is on
        int scale = 2; // default Retina
        CGDirectDisplayID displayIds[16];
        uint32_t displayCount = 0;
        if (CGGetActiveDisplayList(16, displayIds, &displayCount) == kCGErrorSuccess) {
            CGPoint center = CGPointMake(
                targetWindow.frame.origin.x + targetWindow.frame.size.width / 2,
                targetWindow.frame.origin.y + targetWindow.frame.size.height / 2);
            for (uint32_t i = 0; i < displayCount; ++i) {
                CGRect db = CGDisplayBounds(displayIds[i]);
                if (CGRectContainsPoint(db, center)) {
                    size_t pw = CGDisplayPixelsWide(displayIds[i]);
                    scale = (db.size.width > 0) ? static_cast<int>(pw / db.size.width) : 2;
                    break;
                }
            }
        }

        int captureW = static_cast<int>(targetWindow.frame.size.width) * scale;
        int captureH = static_cast<int>(targetWindow.frame.size.height) * scale;

        CGImageRef cgImage = captureWithFilterSync(filter, captureW, captureH);
        [filter release];
        [content release];

        if (!cgImage) return {};

        CaptureImage result = cgImageToCaptureImage(cgImage, scale);
        CGImageRelease(cgImage);
        return result;
    }
}

// ============================================================================
// Permission
// ============================================================================

bool ScreenCaptureUtils_Mac::hasScreenCapturePermission()
{
    return CGPreflightScreenCaptureAccess();
}

void ScreenCaptureUtils_Mac::requestScreenCapturePermission()
{
    CGRequestScreenCaptureAccess();
}

} // namespace ucf::utilities::screencapture

#endif // __APPLE__
