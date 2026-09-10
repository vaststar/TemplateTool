#pragma once

#include "core/AsioErrorMapper.h"
#include "core/ExceptionErrorMapper.h"

#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace ucf::utilities::network::detail {

[[nodiscard]] inline NetworkError makeError(
    NetworkErrorCode code,
    NetworkOperation operation,
    std::string_view diagnostic) noexcept
{
    NetworkError result;
    result.code = code;
    result.operation = operation;
    try
    {
        result.diagnostic = diagnostic;
    }
    catch (...)
    {
    }
    return result;
}

[[nodiscard]] inline NetworkError makePostError(
    const std::system_error& exception,
    NetworkOperation operation) noexcept
{
    NetworkError error = makeNetworkError(exception.code(), operation);
    try
    {
        error.diagnostic = exception.what();
    }
    catch (...)
    {
    }
    return error;
}

[[nodiscard]] inline NetworkError makeCallbackError(const char* callbackName) noexcept
{
    NetworkError result;
    result.code = NetworkErrorCode::CallbackFailed;
    result.operation = NetworkOperation::Callback;
    try
    {
        result.diagnostic = "Network callback failed";
        if (callbackName != nullptr)
        {
            result.diagnostic += ": ";
            result.diagnostic += callbackName;
        }
    }
    catch (...)
    {
    }
    return result;
}

} // namespace ucf::utilities::network::detail
