#pragma once

#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>
#include <ucf/utilities/NetworkTransportUtils/core/TransportCloseInfo.h>

#include <utility>

namespace ucf::utilities::network::detail {

/// Converts a terminal TCP operation error into the public close model. Keeping
/// this policy shared prevents client and accepted-session behavior from
/// drifting, especially for peer failures first observed by a write.
[[nodiscard]] inline TransportCloseInfo makeTcpCloseInfo(NetworkError error) noexcept
{
    const bool streamOperation = error.operation == NetworkOperation::Read
        || error.operation == NetworkOperation::Write;
    const bool peerFailure = error.code == NetworkErrorCode::EndOfStream
        || error.code == NetworkErrorCode::ConnectionReset
        || error.code == NetworkErrorCode::ConnectionAborted
        || error.code == NetworkErrorCode::NotConnected;

    return TransportCloseInfo{
        streamOperation && peerFailure ? CloseOrigin::Peer : CloseOrigin::Error,
        std::move(error),
        0};
}

} // namespace ucf::utilities::network::detail
