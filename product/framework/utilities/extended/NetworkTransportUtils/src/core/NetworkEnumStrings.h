#pragma once

#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>
#include <ucf/utilities/NetworkTransportUtils/core/TransportCloseInfo.h>

#include <string_view>

namespace ucf::utilities::network::detail {

[[nodiscard]] constexpr std::string_view toString(CloseOrigin value) noexcept
{
    switch (value)
    {
    case CloseOrigin::Local:
        return "Local";
    case CloseOrigin::Peer:
        return "Peer";
    case CloseOrigin::RuntimeShutdown:
        return "RuntimeShutdown";
    case CloseOrigin::Error:
        return "Error";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::string_view toString(NetworkOperation value) noexcept
{
    switch (value)
    {
    case NetworkOperation::None:
        return "None";
    case NetworkOperation::Resolve:
        return "Resolve";
    case NetworkOperation::Bind:
        return "Bind";
    case NetworkOperation::Listen:
        return "Listen";
    case NetworkOperation::Accept:
        return "Accept";
    case NetworkOperation::Connect:
        return "Connect";
    case NetworkOperation::Read:
        return "Read";
    case NetworkOperation::Write:
        return "Write";
    case NetworkOperation::Receive:
        return "Receive";
    case NetworkOperation::Send:
        return "Send";
    case NetworkOperation::Shutdown:
        return "Shutdown";
    case NetworkOperation::Close:
        return "Close";
    case NetworkOperation::Callback:
        return "Callback";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::string_view toString(NetworkErrorCode value) noexcept
{
    switch (value)
    {
    case NetworkErrorCode::Unknown:
        return "Unknown";
    case NetworkErrorCode::InvalidArgument:
        return "InvalidArgument";
    case NetworkErrorCode::InvalidState:
        return "InvalidState";
    case NetworkErrorCode::PermissionDenied:
        return "PermissionDenied";
    case NetworkErrorCode::NameResolutionFailed:
        return "NameResolutionFailed";
    case NetworkErrorCode::AddressInUse:
        return "AddressInUse";
    case NetworkErrorCode::AddressNotAvailable:
        return "AddressNotAvailable";
    case NetworkErrorCode::ConnectionRefused:
        return "ConnectionRefused";
    case NetworkErrorCode::ConnectionReset:
        return "ConnectionReset";
    case NetworkErrorCode::ConnectionAborted:
        return "ConnectionAborted";
    case NetworkErrorCode::EndOfStream:
        return "EndOfStream";
    case NetworkErrorCode::NotConnected:
        return "NotConnected";
    case NetworkErrorCode::NetworkUnreachable:
        return "NetworkUnreachable";
    case NetworkErrorCode::HostUnreachable:
        return "HostUnreachable";
    case NetworkErrorCode::TimedOut:
        return "TimedOut";
    case NetworkErrorCode::Cancelled:
        return "Cancelled";
    case NetworkErrorCode::WouldBlock:
        return "WouldBlock";
    case NetworkErrorCode::QueueFull:
        return "QueueFull";
    case NetworkErrorCode::MessageTooLarge:
        return "MessageTooLarge";
    case NetworkErrorCode::ResourceExhausted:
        return "ResourceExhausted";
    case NetworkErrorCode::CallbackFailed:
        return "CallbackFailed";
    case NetworkErrorCode::NotSupported:
        return "NotSupported";
    case NetworkErrorCode::SystemError:
        return "SystemError";
    }
    return "Unknown";
}

} // namespace ucf::utilities::network::detail
