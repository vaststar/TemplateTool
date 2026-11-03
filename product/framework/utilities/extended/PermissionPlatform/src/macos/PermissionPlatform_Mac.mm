#include <ucf/utilities/PermissionPlatform/PermissionPlatform.h>

#import <AppKit/NSWorkspace.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#include <dispatch/dispatch.h>

namespace ucf::utilities::permissions {
namespace {

Capabilities capabilities(Kind kind)
{
    switch (kind)
    {
    case Kind::ScreenCapture:
        if (@available(macOS 10.15, *))
        {
            return { true, true };
        }
        return {};
    case Kind::Microphone:
        if (@available(macOS 10.14, *))
        {
            return { true, true };
        }
        return {};
    }
    return {};
}

Error requestError(const State& state)
{
    if (state.status == Status::Unsupported)
    {
        return Error::Unsupported;
    }
    return state.status == Status::Unknown ? Error::PlatformFailure : Error::None;
}

void finishRequest(Kind kind, const std::function<void(State, Error)>& callback)
{
    const State updated = query(kind);
    if (callback)
    {
        callback(updated, requestError(updated));
    }
}

NSString* settingsAddress(Kind kind)
{
    switch (kind)
    {
    case Kind::ScreenCapture:
        return @"x-apple.systempreferences:com.apple.preference.security?Privacy_ScreenCapture";
    case Kind::Microphone:
        return @"x-apple.systempreferences:com.apple.preference.security?Privacy_Microphone";
    }
    return nil;
}

} // namespace

State query(Kind kind)
{
    State result{ Status::Unknown, capabilities(kind) };
    @autoreleasepool
    {
        switch (kind)
        {
        case Kind::ScreenCapture:
            if (@available(macOS 10.15, *))
            {
                result.status = CGPreflightScreenCaptureAccess()
                    ? Status::Granted : Status::NotGranted;
            }
            else
            {
                result.status = Status::Granted;
            }
            return result;
        case Kind::Microphone:
            if (@available(macOS 10.14, *))
            {
                switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio])
                {
                case AVAuthorizationStatusNotDetermined:
                    result.status = Status::NotDetermined;
                    break;
                case AVAuthorizationStatusRestricted:
                    result.status = Status::Restricted;
                    break;
                case AVAuthorizationStatusDenied:
                    result.status = Status::Denied;
                    break;
                case AVAuthorizationStatusAuthorized:
                    result.status = Status::Granted;
                    break;
                default:
                    result.status = Status::Unknown;
                    break;
                }
            }
            else
            {
                result.status = Status::Granted;
            }
            return result;
        }
    }
    return { Status::Unsupported, {} };
}

void request(Kind kind, std::function<void(State, Error)> callback)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        @autoreleasepool
        {
            const State current = query(kind);
            if (current.status == Status::Granted || !current.capabilities.supportsRequest)
            {
                if (callback)
                {
                    callback(current, current.status == Status::Granted
                        ? Error::None : Error::Unsupported);
                }
                return;
            }

            switch (kind)
            {
            case Kind::ScreenCapture:
                if (@available(macOS 10.15, *))
                {
                    (void)CGRequestScreenCaptureAccess();
                }
                break;
            case Kind::Microphone:
                if (@available(macOS 10.14, *))
                {
                    if (current.status == Status::NotDetermined)
                    {
                        [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio
                            completionHandler:^(BOOL granted) {
                                (void)granted;
                                dispatch_async(dispatch_get_main_queue(), ^{
                                    finishRequest(kind, callback);
                                });
                            }];
                        return;
                    }
                }
                break;
            }
            finishRequest(kind, callback);
        }
    });
}

void openSettings(Kind kind, std::function<void(Error)> callback)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        @autoreleasepool
        {
            if (!capabilities(kind).supportsSettings)
            {
                if (callback) callback(Error::Unsupported);
                return;
            }

            // The pane URLs are best-effort system links. Fall back to the
            // privacy pane when the OS cannot open the requested category.
            NSURL* url = [NSURL URLWithString:settingsAddress(kind)];
            NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
            if (url && [workspace openURL:url])
            {
                if (callback) callback(Error::None);
                return;
            }
            NSURL* fallback = [NSURL URLWithString:
                @"x-apple.systempreferences:com.apple.preference.security"];
            const Error error = fallback && [workspace openURL:fallback]
                ? Error::None : Error::PlatformFailure;
            if (callback) callback(error);
        }
    });
}

} // namespace ucf::utilities::permissions
