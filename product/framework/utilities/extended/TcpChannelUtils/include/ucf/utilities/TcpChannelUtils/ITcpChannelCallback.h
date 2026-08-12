#pragma once

#include <string>

namespace ucf::utilities {

/// Callback interface for TcpChannel events.
///
/// All callbacks are invoked on the TcpChannel's internal I/O thread.
/// Implementations must ensure thread safety when accessing shared state.
class ITcpChannelCallback
{
public:
    virtual ~ITcpChannelCallback() = default;

    /// A client has connected to the server.
    virtual void onClientConnected() = 0;

    /// The connected client has disconnected.
    virtual void onClientDisconnected() = 0;

    /// Raw bytes received from the client.
    /// Data may not be a complete message — caller is responsible for framing.
    /// @param data  Raw bytes received
    virtual void onDataReceived(const std::string& data) = 0;

    /// An error occurred (e.g. socket failure).
    /// @param errorMessage  Human-readable error description
    virtual void onError(const std::string& errorMessage) = 0;
};

} // namespace ucf::utilities
