#pragma once

#include "core/ExceptionErrorMapper.h"
#include "core/NetworkErrorUtils.h"

#include <ucf/utilities/NetworkTransportUtils/core/Endpoint.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkResult.h>

#include <string>
#include <system_error>
#include <utility>

namespace ucf::utilities::network::detail {

/// Converts any Asio IP endpoint without leaking its protocol type into public
/// headers. TCP and UDP intentionally share this tiny value adapter only.
template<typename NativeEndpoint>
[[nodiscard]] NetworkResult<Endpoint> toPublicEndpoint(
    const NativeEndpoint& endpoint,
    NetworkOperation operation)
{
    try
    {
        std::string host = endpoint.address().to_string();
        return NetworkResult<Endpoint>::success(Endpoint{std::move(host), endpoint.port()});
    }
    catch (const std::system_error& exception)
    {
        return NetworkResult<Endpoint>::failure(makePostError(exception, operation));
    }
    catch (const std::exception& exception)
    {
        return NetworkResult<Endpoint>::failure(makeExceptionError(exception, operation));
    }
    catch (...)
    {
        return NetworkResult<Endpoint>::failure(makeUnknownExceptionError(operation));
    }
}

} // namespace ucf::utilities::network::detail
