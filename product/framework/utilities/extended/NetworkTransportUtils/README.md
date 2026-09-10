# NetworkTransportUtils

`NetworkTransportUtils` is an asynchronous TCP/UDP transport layer that keeps
Asio out of business-facing headers. It provides bounded queues, structured
errors, deterministic terminal events, and a shared execution runtime.

## Basic model

```cpp
#include <ucf/utilities/NetworkTransportUtils/NetworkTransportUtils.h>

using namespace ucf::utilities::network;

NetworkRuntime runtime{{.ioThreadCount = 2}};
TcpClient client{runtime};
TcpServer server{runtime};
UdpSocket socket{runtime};
```

A command returning success means that the transport accepted ownership of the
operation or buffer. It does not mean that asynchronous DNS, bind, connect, or
delivery has completed. Completion is reported through callbacks.

The runtime facade may be destroyed before its transports. Active transports
retain a private runtime-core lease and are closed gracefully with
`CloseOrigin::RuntimeShutdown`. `shutdown()` waits until their terminal
callbacks have drained; it must not be called from one of the runtime's own I/O
callbacks. `requestShutdown()` is the callback-safe, non-blocking alternative.

## Lifecycle contract

While its transport object remains alive, every accepted lifecycle has exactly
one terminal callback:

- `TcpClient::connect()` -> `TcpClientHandlers::onClosed`
- `TcpServer::start()` -> `TcpServerHandlers::onStopped`
- `UdpSocket::bind()` -> `UdpSocketHandlers::onClosed`

Object destruction is the deliberate exception: it disables subsequent callback
dispatch and still schedules actor-owned cleanup. A callback already being
dispatched may finish.

`TransportCloseInfo` identifies whether the lifecycle ended locally, at the
peer, during runtime shutdown, or because of an error. `cause` contains the
structured error when applicable. `discardedWriteBytes` is the number of bytes
accepted by `send()`/`sendTo()` but not confirmed written before termination.
If close, shutdown, and an I/O or callback failure race, the first terminal path
to claim the lifecycle determines the reported origin and cause.

Objects are reusable after their terminal callback starts: their public state
has already returned to `Idle`, so a new lifecycle can be started from that
callback.

## TCP client

```cpp
TcpClientHandlers handlers;
handlers.onConnected = [] {
    // Connection is ready.
};
handlers.onDataReceived = [](ByteView chunk) {
    consumeStreamBytes(chunk);
};
handlers.onClosed = [](const TransportCloseInfo& info) {
    if (info.cause)
        report(info.cause.value());
};

TcpClient client{runtime, std::move(handlers)};
if (auto accepted = client.connect({"example.com", 9000}); !accepted)
    report(accepted.error());
```

TCP is a byte stream: one callback can contain part of an application message
or several messages. Framing belongs above this layer. Writes preserve command
order and are admitted against `maxQueuedWriteBytes`. `connectTimeout` covers
DNS and connection establishment together. `noDelay` and `keepAlive` expose the
common socket policies without exposing native socket types.
Numeric destination addresses bypass DNS resolution.

## TCP server

```cpp
TcpServerHandlers handlers;
handlers.onListening = [](const Endpoint& bound) {
    publishListeningAddress(bound);
};
handlers.onConnected = [](const TcpConnectionInfo& connection) {
    remember(connection.id, connection.remoteEndpoint);
};
handlers.onDataReceived = [](TcpConnectionId id, ByteView chunk) {
    consumeClientBytes(id, chunk);
};
handlers.onConnectionClosed = [](TcpConnectionId id, const TransportCloseInfo& info) {
    forget(id, info);
};
handlers.onStopped = [](const TransportCloseInfo& info) {
    observeServerStop(info);
};

TcpServer server{runtime, std::move(handlers)};
if (auto accepted = server.start({"0.0.0.0", 9000}); !accepted)
    report(accepted.error());
```

`start()` resolves and binds asynchronously; `onListening` reports the actual
address, including an OS-selected port. Every public server callback is routed
through one event strand, so callbacks never overlap and preserve causal order.
Listener control and each accepted connection use separate strands, so a slow
business callback does not create socket data races.

Inbound TCP data uses a one-buffer handoff: the session pauses its next read,
hands the owning buffer to the event strand, invokes the callback, and only then
returns the buffer to the session actor. This avoids a copy while bounding
pending inbound data to one chunk per connection. Outbound data is bounded both
per connection and across the whole server.

## UDP socket

```cpp
UdpSocketHandlers handlers;
handlers.onBound = [](const Endpoint& bound) {
    publishLocalAddress(bound);
};
handlers.onDatagramReceived = [](const Endpoint& sender, ByteView datagram) {
    consumeDatagram(sender, datagram);
};
handlers.onSendError = [](const Endpoint& destination, const NetworkError& error) {
    reportSend(destination, error);
};
handlers.onClosed = [](const TransportCloseInfo& info) {
    observeSocketClose(info);
};

UdpSocket socket{runtime, std::move(handlers)};
if (auto accepted = socket.bind(); !accepted)
    report(accepted.error());
```

UDP preserves datagram boundaries and permits empty datagrams. Numeric
destinations bypass DNS; successful hostname resolutions are cached for
`dnsCacheTtl` within `maxDnsCacheEntries`. Send failures are non-terminal and
later queued datagrams continue. Oversized receives are detected portably with a
one-byte probe rather than relying on OS-specific truncation errors.

## Callback and error rules

Callbacks are serialized per public transport object and may call that same
object's thread-safe commands. They should return quickly. A callback exception
never escapes into `io_context::run()`:

- failures in connected/data/non-terminal error callbacks close the affected
  transport with `NetworkErrorCode::CallbackFailed`;
- exceptions in an already-terminal callback are logged and contained.

`ByteView` and callback endpoint references are valid only during the callback.
Copy a view into `ByteBuffer` when data must be retained.

Use `NetworkError::code` and `operation` for decisions. `nativeError` and
`diagnostic` are troubleshooting context and must not be parsed by business
logic. Queue-full results are synchronous and are the signal to apply upstream
backpressure.

See [ARCHITECTURE.md](ARCHITECTURE.md) for actor ownership, shutdown, ordering,
and extension invariants.
