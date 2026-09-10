#pragma once

#include "NetworkTransportLogger.h"

#include <exception>
#include <functional>
#include <utility>

namespace ucf::utilities::network::detail {

/// Invokes a user callback at the transport boundary. Returning false lets the
/// owner turn a callback exception into a deterministic terminal event instead
/// of allowing it to escape through asio::io_context::run().
template<typename Callback, typename... Arguments>
[[nodiscard]] bool invokeCallback(
    const char* logTag,
    const char* callbackName,
    const Callback& callback,
    Arguments&&... arguments) noexcept
{
    if (!callback)
    {
        return true;
    }

    try
    {
        std::invoke(callback, std::forward<Arguments>(arguments)...);
        return true;
    }
    catch (const std::exception& exception)
    {
        NETWORK_TRANSPORT_LOG_ERROR(logTag, callbackName << " callback threw: " << exception.what());
    }
    catch (...)
    {
        NETWORK_TRANSPORT_LOG_ERROR(logTag, callbackName << " callback threw an unknown exception");
    }
    return false;
}

} // namespace ucf::utilities::network::detail
