#pragma once

#include <functional>

#include <ucf/utilities/PermissionPlatform/PermissionPlatformExport.h>

namespace ucf::utilities::permissions {

enum class Kind
{
    ScreenCapture,
    Microphone
};

enum class Status
{
    Unknown,
    NotDetermined,
    NotGranted,
    Granted,
    Limited,
    Denied,
    Restricted,
    Unsupported
};

enum class Error
{
    None,
    Unsupported,
    PlatformFailure
};

struct PERMISSION_PLATFORM_API Capabilities
{
    bool supportsRequest = false;
    bool supportsSettings = false;
};

struct PERMISSION_PLATFORM_API State
{
    Status status = Status::Unknown;
    Capabilities capabilities;
};

/// Read current native state without prompting or caching. Safe from any thread.
/// Screen capture reports NotGranted when macOS cannot distinguish its reason.
/// Unsupported platforms report Unsupported; this is not a denial of capture.
PERMISSION_PLATFORM_API State query(Kind kind);

/// Request access using the native permission API. Request denial is a normal
/// result (Error::None), distinct from a failure to invoke the platform API.
/// A non-empty callback is invoked exactly once, including immediate failures.
/// Completion may be synchronous or asynchronous. Callbacks must not throw.
/// macOS dispatches native requests and completion onto the main queue.
PERMISSION_PLATFORM_API void request(Kind kind,
    std::function<void(State, Error)> callback);

/// Ask the system to open the relevant privacy settings page. Error::None means
/// the open request was accepted, not that access was granted or a particular
/// pane is visible. Completion follows the same contract as request().
PERMISSION_PLATFORM_API void openSettings(Kind kind,
    std::function<void(Error)> callback);

} // namespace ucf::utilities::permissions
