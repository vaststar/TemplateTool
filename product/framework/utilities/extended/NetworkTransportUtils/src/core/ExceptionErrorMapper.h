#pragma once

#include "core/AsioErrorMapper.h"

#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>

#include <exception>
#include <new>
#include <system_error>
#include <utility>

namespace ucf::utilities::network {

[[nodiscard]] inline NetworkError makeExceptionError(const std::exception& exception, NetworkOperation operation) noexcept
{
    NetworkError result;
    result.operation = operation;
    if (const auto* systemError = dynamic_cast<const std::system_error*>(&exception))
    {
        result = makeNetworkError(systemError->code(), operation);
    }
    else
    {
        result.code = dynamic_cast<const std::bad_alloc*>(&exception) != nullptr
            ? NetworkErrorCode::ResourceExhausted
            : NetworkErrorCode::SystemError;
    }
    try
    {
        result.diagnostic = exception.what();
    }
    catch (...)
    {
    }
    return result;
}

[[nodiscard]] inline NetworkError makeUnknownExceptionError(NetworkOperation operation) noexcept
{
    NetworkError result;
    result.code = NetworkErrorCode::SystemError;
    result.operation = operation;
    try
    {
        result.diagnostic = "Unknown exception in an asynchronous network operation";
    }
    catch (...)
    {
    }
    return result;
}

} // namespace ucf::utilities::network
