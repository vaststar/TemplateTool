#include "core/AsioErrorMapper.h"

#include <asio/error.hpp>

#include <system_error>

namespace ucf::utilities::network {

namespace {

NetworkErrorCode mapErrorCode(const asio::error_code& error, NetworkOperation operation)
{
    if (!error)
    {
        return NetworkErrorCode::Unknown;
    }

    if (error == asio::error::operation_aborted)
    {
        return NetworkErrorCode::Cancelled;
    }

    if (error == asio::error::invalid_argument || error == asio::error::name_too_long)
    {
        return NetworkErrorCode::InvalidArgument;
    }

    if (error == asio::error::access_denied || error == asio::error::no_permission)
    {
        return NetworkErrorCode::PermissionDenied;
    }

    if (error == asio::error::address_in_use)
    {
        return NetworkErrorCode::AddressInUse;
    }

    if (error == std::errc::address_not_available)
    {
        return NetworkErrorCode::AddressNotAvailable;
    }

    if (error == asio::error::connection_refused)
    {
        return NetworkErrorCode::ConnectionRefused;
    }

    if (error == asio::error::eof)
    {
        return NetworkErrorCode::EndOfStream;
    }

    if (error == asio::error::connection_reset
        || error == asio::error::network_reset
        || error == asio::error::broken_pipe
        || error == asio::error::shut_down)
    {
        return NetworkErrorCode::ConnectionReset;
    }

    if (error == asio::error::connection_aborted)
    {
        return NetworkErrorCode::ConnectionAborted;
    }

    if (error == asio::error::not_connected)
    {
        return NetworkErrorCode::NotConnected;
    }

    if (error == asio::error::network_down || error == asio::error::network_unreachable)
    {
        return NetworkErrorCode::NetworkUnreachable;
    }

    if (error == asio::error::host_unreachable)
    {
        return NetworkErrorCode::HostUnreachable;
    }

    if (error == asio::error::timed_out)
    {
        return NetworkErrorCode::TimedOut;
    }

    if (error == asio::error::try_again || error == asio::error::would_block)
    {
        return NetworkErrorCode::WouldBlock;
    }

    if (error == asio::error::message_size)
    {
        return NetworkErrorCode::MessageTooLarge;
    }

    if (error == asio::error::no_descriptors || error == asio::error::no_buffer_space || error == asio::error::no_memory)
    {
        return NetworkErrorCode::ResourceExhausted;
    }

    if (error == asio::error::address_family_not_supported
        || error == asio::error::no_protocol_option
        || error == asio::error::operation_not_supported
        || error == asio::error::socket_type_not_supported)
    {
        return NetworkErrorCode::NotSupported;
    }

    if (error == asio::error::already_connected
        || error == asio::error::already_started
        || error == asio::error::in_progress
        || error == asio::error::already_open)
    {
        return NetworkErrorCode::InvalidState;
    }

    if (operation == NetworkOperation::Resolve
        || error == asio::error::host_not_found
        || error == asio::error::host_not_found_try_again
        || error == asio::error::no_data
        || error == asio::error::no_recovery
        || error == asio::error::service_not_found)
    {
        return NetworkErrorCode::NameResolutionFailed;
    }

    return NetworkErrorCode::SystemError;
}

} // namespace

NetworkError makeNetworkError(const asio::error_code& error, NetworkOperation operation) noexcept
{
    NetworkError result;
    result.code = mapErrorCode(error, operation);
    result.operation = operation;
    result.nativeError = error;
    try
    {
        result.diagnostic = error.message();
    }
    catch (...)
    {
    }
    return result;
}

} // namespace ucf::utilities::network
