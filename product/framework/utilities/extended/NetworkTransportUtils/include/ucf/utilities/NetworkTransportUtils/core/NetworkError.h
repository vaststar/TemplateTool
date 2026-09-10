#pragma once

#include <string>
#include <system_error>

namespace ucf::utilities::network {

/// Stable error categories for decisions made by callers.
enum class NetworkErrorCode
{
    Unknown,
    InvalidArgument,
    InvalidState,
    PermissionDenied,
    NameResolutionFailed,
    AddressInUse,
    AddressNotAvailable,
    ConnectionRefused,
    ConnectionReset,
    ConnectionAborted,
    EndOfStream,
    NotConnected,
    NetworkUnreachable,
    HostUnreachable,
    TimedOut,
    Cancelled,
    WouldBlock,
    QueueFull,
    MessageTooLarge,
    ResourceExhausted,
    CallbackFailed,
    NotSupported,
    SystemError
};

/// Identifies the operation that produced a NetworkError.
enum class NetworkOperation
{
    None,
    Resolve,
    Bind,
    Listen,
    Accept,
    Connect,
    Read,
    Write,
    Receive,
    Send,
    Shutdown,
    Close,
    Callback
};

struct NetworkError final
{
    // Use code for stable program decisions. nativeError and diagnostic are
    // troubleshooting context and must not be parsed by business code.
    NetworkErrorCode code{NetworkErrorCode::Unknown};
    NetworkOperation operation{NetworkOperation::None};
    std::error_code nativeError;
    std::string diagnostic;
};

} // namespace ucf::utilities::network
