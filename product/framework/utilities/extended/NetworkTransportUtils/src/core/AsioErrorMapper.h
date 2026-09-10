#pragma once

#include <ucf/utilities/NetworkTransportUtils/NetworkTransportUtilsExport.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>

#include <asio/error_code.hpp>

namespace ucf::utilities::network {

/// Converts implementation-specific Asio errors into the stable public model.
[[nodiscard]] NETWORK_TRANSPORT_UTILS_API NetworkError makeNetworkError(
    const asio::error_code& error,
    NetworkOperation operation) noexcept;

} // namespace ucf::utilities::network
