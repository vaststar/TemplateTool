#pragma once

#include <ucf/utilities/NetworkTransportUtils/NetworkTransportUtilsExport.h>
#include <ucf/utilities/NetworkTransportUtils/core/ByteBuffer.h>
#include <ucf/utilities/NetworkTransportUtils/core/Endpoint.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkResult.h>
#include <ucf/utilities/NetworkTransportUtils/core/TransportCloseInfo.h>

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>

namespace ucf::utilities::network {

class NetworkRuntime;

enum class TcpClientState
{
    Idle,
    Connecting,
    Connected,
    Closing
};

struct TcpClientOptions final
{
    // Size of the reusable buffer owned by each connection attempt.
    std::size_t readBufferSize{16 * 1024};
    // Includes both queued data and the write currently in progress.
    std::size_t maxQueuedWriteBytes{4 * 1024 * 1024};
    // Covers DNS resolution and connection establishment. Zero disables it.
    std::chrono::milliseconds connectTimeout{30'000};
    bool noDelay{true};
    bool keepAlive{false};
};

struct TcpClientHandlers final
{
    std::function<void()> onConnected;
    // TCP is a byte stream: each callback contains an arbitrary chunk rather
    // than one application message. The view is valid only during callback.
    std::function<void(ByteView)> onDataReceived;
    // Unless the client is destroyed, emitted exactly once for every accepted
    // connect() lifecycle, including establishment failure, peer EOF, local
    // close, and runtime shutdown.
    std::function<void(const TransportCloseInfo&)> onClosed;
};

/// Owns one reusable asynchronous TCP connection.
///
/// All callbacks for one client are serialized on its actor strand and should
/// return quickly. Public commands are thread-safe and may also be called from
/// a callback. Runtime shutdown closes active clients gracefully. Destruction
/// disables subsequent callback dispatch without blocking; a callback already
/// being dispatched may finish.
class NETWORK_TRANSPORT_UTILS_API TcpClient final
{
public:
    explicit TcpClient(NetworkRuntime& runtime, TcpClientHandlers handlers = {});
    TcpClient(NetworkRuntime& runtime, TcpClientOptions options, TcpClientHandlers handlers = {});
    ~TcpClient();

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;
    TcpClient(TcpClient&&) = delete;
    TcpClient& operator=(TcpClient&&) = delete;

    /// Starts DNS resolution and connection establishment. The configured
    /// timeout covers both phases and reports NetworkErrorCode::TimedOut.
    ///
    /// A successful result means the command was accepted. onConnected reports
    /// success; onClosed reports every terminal outcome. A new connection may
    /// be started from onClosed or any time after it returns.
    [[nodiscard]] NetworkResult<void> connect(Endpoint endpoint);

    /// Queues bytes for ordered asynchronous delivery.
    ///
    /// A successful result means this client owns the buffer and accepted it
    /// into the bounded write queue. It does not mean the peer received it.
    [[nodiscard]] NetworkResult<void> send(ByteBuffer data);

    /// Cancels connection establishment or closes the active connection.
    /// This operation is non-blocking and idempotent. An active accepted
    /// connect() lifecycle still emits its one onClosed event; object destruction
    /// suppresses that callback.
    void close() noexcept;

    [[nodiscard]] TcpClientState state() const noexcept;

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;
};

} // namespace ucf::utilities::network
