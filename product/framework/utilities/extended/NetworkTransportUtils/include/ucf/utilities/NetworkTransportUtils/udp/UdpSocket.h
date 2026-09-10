#pragma once

#include <ucf/utilities/NetworkTransportUtils/NetworkTransportUtilsExport.h>
#include <ucf/utilities/NetworkTransportUtils/core/ByteBuffer.h>
#include <ucf/utilities/NetworkTransportUtils/core/Endpoint.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkResult.h>
#include <ucf/utilities/NetworkTransportUtils/core/TransportCloseInfo.h>

#include <cstddef>
#include <chrono>
#include <functional>
#include <memory>

namespace ucf::utilities::network {

class NetworkRuntime;

enum class UdpSocketState
{
    Idle,
    Binding,
    Bound,
    Closing
};

struct UdpSocketOptions final
{
    // Portable upper limit used for both incoming and outgoing datagrams.
    std::size_t maxDatagramSize{65'507};
    // Includes datagrams already accepted and the send currently in progress.
    std::size_t maxQueuedSendBytes{4 * 1024 * 1024};
    // Also bounds zero-length datagrams, which do not consume byte capacity.
    std::size_t maxQueuedDatagrams{1024};
    // Successful hostname resolutions are reused to avoid serial DNS work on
    // repeated sends. Zero disables caching. Numeric addresses bypass DNS.
    std::chrono::milliseconds dnsCacheTtl{60'000};
    // Bounds cache memory across many distinct destinations. Zero disables
    // caching without changing dnsCacheTtl.
    std::size_t maxDnsCacheEntries{256};
    bool reuseAddress{false};
};

struct UdpSocketHandlers final
{
    // bind() is asynchronous. This reports the address actually bound.
    std::function<void(const Endpoint&)> onBound;
    // UDP preserves datagram boundaries. The view and sender endpoint are
    // valid only during this callback.
    std::function<void(const Endpoint&, ByteView)> onDatagramReceived;
    // DNS and asynchronous send failures are non-terminal. The first argument
    // is the destination originally supplied to sendTo().
    std::function<void(const Endpoint&, const NetworkError&)> onSendError;
    // Only non-terminal receive failures are reported here.
    std::function<void(const NetworkError&)> onReceiveError;
    // Unless the socket is destroyed, emitted exactly once for every accepted
    // bind() lifecycle. Bind and fatal receive failures are carried in cause.
    std::function<void(const TransportCloseInfo&)> onClosed;
};

/// Owns one reusable asynchronous UDP socket.
///
/// UDP has no connection or accepted-session concept, so the same object is
/// used by requesters, responders, and peer-to-peer code. Callbacks are
/// serialized on the socket actor strand and should return quickly. Public
/// commands are thread-safe and callback-safe. Destruction disables subsequent
/// callback dispatch and schedules actor-owned cleanup without blocking; a
/// callback already being dispatched may finish.
class NETWORK_TRANSPORT_UTILS_API UdpSocket final
{
public:
    explicit UdpSocket(NetworkRuntime& runtime, UdpSocketHandlers handlers = {});
    UdpSocket(NetworkRuntime& runtime, UdpSocketOptions options, UdpSocketHandlers handlers = {});
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;
    UdpSocket(UdpSocket&&) = delete;
    UdpSocket& operator=(UdpSocket&&) = delete;

    /// Asynchronously resolves and binds the socket. An empty host binds all
    /// IPv4 interfaces, and port zero lets the OS select an available port.
    /// Success means the command was accepted; onBound reports the actual
    /// endpoint. Bind "::" explicitly when an IPv6 socket is required.
    [[nodiscard]] NetworkResult<void> bind(Endpoint localEndpoint = {});

    /// Queues one complete datagram for asynchronous delivery. Host names are
    /// resolved internally. Success means the bounded queue accepted the data,
    /// not that the destination received it. Zero-length datagrams are valid.
    [[nodiscard]] NetworkResult<void> sendTo(Endpoint destination, ByteBuffer data);

    /// Cancels pending work and closes the socket. This operation is
    /// non-blocking and idempotent. onClosed is emitted once for each accepted
    /// bind() lifecycle, including asynchronous bind failure, but not during
    /// object destruction.
    void close() noexcept;

    [[nodiscard]] UdpSocketState state() const noexcept;

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;
};

} // namespace ucf::utilities::network
