#pragma once

namespace commonHead::viewModels::model {

enum class PermissionKind { ScreenCapture, Microphone };
enum class PermissionStatus
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
enum class PermissionAction { None, Request, OpenSettings };
enum class PermissionError { None, Unsupported, QueryFailed, RequestFailed, SettingsOpenFailed };

struct PermissionState
{
    PermissionStatus status = PermissionStatus::Unknown;
    PermissionAction action = PermissionAction::None;
    bool busy = false;
    PermissionError error = PermissionError::None;
};

} // namespace commonHead::viewModels::model
