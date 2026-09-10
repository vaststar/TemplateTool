#pragma once

#include <ucf/utilities/NetworkTransportUtils/NetworkTransportUtilsExport.h>
#include <ucf/utilities/NetworkTransportUtils/core/ByteBuffer.h>
#include <ucf/utilities/NetworkTransportUtils/core/Endpoint.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkResult.h>
#include <ucf/utilities/NetworkTransportUtils/core/TransportCloseInfo.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

namespace ucf::utilities::network {

class NetworkRuntime;

using TcpConnectionId = std::uint64_t;

enum class TcpServerState
{
    Idle,
    Starting,
    Listening,
    Stopping
};

struct TcpConnectionInfo final
{
    TcpConnectionId id{0};
    Endpoint localEndpoint;
    Endpoint remoteEndpoint;
};

struct TcpServerOptions final
{
    // Operating-system queue for connections waiting to be accepted.
    std::size_t listenBacklog{128};
    // Accepting pauses at this limit and resumes when a connection closes.
    std::size_t maxConnections{128};
    std::size_t readBufferSize{16 * 1024};
    // Includes queued data and the write currently in progress.
    std::size_t maxQueuedWriteBytesPerConnection{4 * 1024 * 1024};
    // Bounds all accepted but not-yet-written bytes across the server.
    std::size_t maxTotalQueuedWriteBytes{64 * 1024 * 1024};
    bool noDelay{true};
    bool keepAlive{false};
    bool reuseAddress{true};
};

struct TcpServerHandlers final
{
    // start() is asynchronous. This reports the address actually bound.
    std::function<void(const Endpoint&)> onListening;
    std::function<void(const TcpConnectionInfo&)> onConnected;
    // TCP is a byte stream: each callback contains an arbitrary chunk rather
    // than one application message. The view is valid only during callback.
    std::function<void(TcpConnectionId, ByteView)> onDataReceived;
    // Emitted exactly once for every accepted connection.
    std::function<void(TcpConnectionId, const TransportCloseInfo&)> onConnectionClosed;
    // Reports recoverable listener errors. Fatal listener/start errors are
    // carried by onStopped.
    std::function<void(const NetworkError&)> onServerError;
    // Unless the server is destroyed, emitted exactly once for every accepted
    // start() lifecycle. A start failure has origin == CloseOrigin::Error.
    std::function<void(const TransportCloseInfo&)> onStopped;
};

/// Owns one reusable asynchronous TCP listener and its accepted connections.
///
/// Every public callback for one server is dispatched through one dedicated
/// event strand. Callbacks therefore never overlap and preserve causal order,
/// while listener and per-session I/O continue on separate actor strands.
/// Public commands are thread-safe and may also be called from a callback.
/// Destruction disables subsequent callback dispatch and schedules actor-owned
/// cleanup without blocking; a callback already being dispatched may finish.
class NETWORK_TRANSPORT_UTILS_API TcpServer final
{
public:
    explicit TcpServer(NetworkRuntime& runtime, TcpServerHandlers handlers = {});
    TcpServer(NetworkRuntime& runtime, TcpServerOptions options, TcpServerHandlers handlers = {});
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;
    TcpServer(TcpServer&&) = delete;
    TcpServer& operator=(TcpServer&&) = delete;

    /// Asynchronously resolves, binds, and starts the listener. An empty host
    /// binds all IPv4 interfaces, and port zero lets the OS select a port.
    /// Success means the command was accepted; onListening reports the bound
    /// endpoint and onStopped reports a start failure.
    [[nodiscard]] NetworkResult<void> start(Endpoint bindEndpoint);

    /// Queues bytes for ordered asynchronous delivery to one connection.
    [[nodiscard]] NetworkResult<void> send(TcpConnectionId connectionId, ByteBuffer data);

    /// Closes one connection. Unknown and already closed IDs are ignored.
    void disconnect(TcpConnectionId connectionId) noexcept;

    /// Stops accepting and closes every connection. This operation is
    /// non-blocking and idempotent. Every onConnectionClosed callback is
    /// delivered before onStopped reports completion.
    void stop() noexcept;

    [[nodiscard]] TcpServerState state() const noexcept;
    [[nodiscard]] std::size_t connectionCount() const noexcept;

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;
};

} // namespace ucf::utilities::network
