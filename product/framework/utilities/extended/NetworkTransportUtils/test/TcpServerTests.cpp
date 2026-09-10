#include <catch2/catch_test_macros.hpp>

#include <ucf/utilities/NetworkTransportUtils/runtime/NetworkRuntime.h>
#include <ucf/utilities/NetworkTransportUtils/tcp/TcpServer.h>

#include "NetworkTestSupport.h"
#include "runtime/NetworkRuntimeAccess.h"

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <future>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using namespace std::chrono_literals;
using asio::ip::tcp;
using network_test::EventCounter;
using network_test::OneShot;
using network_test::bytes;
using ucf::utilities::network::ByteBuffer;
using ucf::utilities::network::ByteView;
using ucf::utilities::network::CloseOrigin;
using ucf::utilities::network::Endpoint;
using ucf::utilities::network::NetworkError;
using ucf::utilities::network::NetworkErrorCode;
using ucf::utilities::network::NetworkRuntime;
using ucf::utilities::network::NetworkRuntimeOptions;
using ucf::utilities::network::TcpConnectionId;
using ucf::utilities::network::TcpConnectionInfo;
using ucf::utilities::network::TcpServer;
using ucf::utilities::network::TcpServerHandlers;
using ucf::utilities::network::TcpServerOptions;
using ucf::utilities::network::TcpServerState;
using ucf::utilities::network::TransportCloseInfo;
using ucf::utilities::network::detail::NetworkRuntimeAccess;

class CallbackProbe final
{
public:
    void enter(std::string event)
    {
        const int active = mActive.fetch_add(1) + 1;
        int observed = mMaxActive.load();
        while (active > observed && !mMaxActive.compare_exchange_weak(observed, active))
        {
        }
        {
            std::scoped_lock lock{mMutex};
            mEvents.emplace_back(std::move(event));
        }
        std::this_thread::sleep_for(5ms);
        mActive.fetch_sub(1);
        mCondition.notify_all();
    }

    [[nodiscard]] bool waitForSize(std::size_t size)
    {
        std::unique_lock lock{mMutex};
        return mCondition.wait_for(lock, 3s, [this, size] { return mEvents.size() >= size; });
    }

    [[nodiscard]] int maxActive() const noexcept { return mMaxActive.load(); }

    [[nodiscard]] std::vector<std::string> events() const
    {
        std::scoped_lock lock{mMutex};
        return mEvents;
    }

private:
    std::atomic<int> mActive{0};
    std::atomic<int> mMaxActive{0};
    mutable std::mutex mMutex;
    std::condition_variable mCondition;
    std::vector<std::string> mEvents;
};

[[nodiscard]] tcp::endpoint nativeEndpoint(const Endpoint& endpoint)
{
    return tcp::endpoint{asio::ip::make_address(endpoint.host), endpoint.port};
}

[[nodiscard]] ByteBuffer readExactly(tcp::socket& socket, std::size_t size)
{
    ByteBuffer result(size);
    asio::read(socket, asio::buffer(result));
    return result;
}

} // namespace

TEST_CASE("TcpServer accepts asynchronously and send from onConnected is actor-safe", "[NetworkTransportUtils][TcpServer]")
{
    NetworkRuntimeOptions runtimeOptions;
    runtimeOptions.ioThreadCount = 4;
    NetworkRuntime runtime{runtimeOptions};
    OneShot<Endpoint> listening;
    OneShot<TcpConnectionInfo> connected;
    OneShot<TransportCloseInfo> connectionClosed;
    OneShot<TransportCloseInfo> stopped;
    TcpServer* serverAddress{nullptr};

    TcpServerHandlers handlers;
    handlers.onListening = [&listening](const Endpoint& endpoint) { listening.set(endpoint); };
    handlers.onConnected = [&connected, &serverAddress](const TcpConnectionInfo& info) {
        connected.set(info);
        const auto result = serverAddress->send(info.id, bytes("hello"));
        if (!result)
        {
            throw std::runtime_error{"send from onConnected was rejected"};
        }
    };
    handlers.onDataReceived = [&serverAddress](TcpConnectionId id, ByteView data) {
        const auto result = serverAddress->send(id, ByteBuffer{data.begin(), data.end()});
        if (!result)
        {
            throw std::runtime_error{"echo send was rejected"};
        }
    };
    handlers.onConnectionClosed = [&connectionClosed](TcpConnectionId, const TransportCloseInfo& info) {
        connectionClosed.set(info);
    };
    handlers.onStopped = [&stopped](const TransportCloseInfo& info) { stopped.set(info); };

    TcpServer server{runtime, std::move(handlers)};
    serverAddress = &server;
    REQUIRE(server.start(Endpoint{"127.0.0.1", 0}));
    REQUIRE(server.state() != TcpServerState::Idle);
    REQUIRE(listening.wait());
    const Endpoint bound = listening.get();
    REQUIRE(bound.port != 0);

    asio::io_context clientContext;
    tcp::socket client{clientContext};
    client.connect(nativeEndpoint(bound));
    REQUIRE(connected.wait());
    const TcpConnectionInfo info = connected.get();
    REQUIRE(info.localEndpoint == bound);
    REQUIRE(info.remoteEndpoint.port != 0);
    REQUIRE(readExactly(client, 5) == bytes("hello"));
    asio::write(client, asio::buffer(bytes("ping")));
    REQUIRE(readExactly(client, 4) == bytes("ping"));

    asio::error_code ignoredError;
    client.close(ignoredError);
    REQUIRE(connectionClosed.wait());
    REQUIRE(connectionClosed.get().origin == CloseOrigin::Peer);
    REQUIRE(server.connectionCount() == 0);
    server.stop();
    REQUIRE(stopped.wait());
    REQUIRE(stopped.get().origin == CloseOrigin::Local);
    runtime.shutdown();
}

TEST_CASE("TcpServer serializes every public callback on one event strand", "[NetworkTransportUtils][TcpServer][Concurrency]")
{
    NetworkRuntimeOptions options;
    options.ioThreadCount = 4;
    NetworkRuntime runtime{options};
    CallbackProbe probe;
    OneShot<Endpoint> listening;
    EventCounter connected;
    EventCounter dataReceived;
    EventCounter connectionClosed;
    OneShot<void> stopped;

    TcpServerHandlers handlers;
    handlers.onListening = [&probe, &listening](const Endpoint& endpoint) {
        probe.enter("listening");
        listening.set(endpoint);
    };
    handlers.onConnected = [&probe, &connected](const TcpConnectionInfo& info) {
        probe.enter("connected:" + std::to_string(info.id));
        connected.signal();
    };
    handlers.onDataReceived = [&probe, &dataReceived](TcpConnectionId id, ByteView) {
        probe.enter("data:" + std::to_string(id));
        dataReceived.signal();
    };
    handlers.onConnectionClosed = [&probe, &connectionClosed](TcpConnectionId id, const TransportCloseInfo&) {
        probe.enter("closed:" + std::to_string(id));
        connectionClosed.signal();
    };
    handlers.onServerError = [&probe](const NetworkError&) { probe.enter("server-error"); };
    handlers.onStopped = [&probe, &stopped](const TransportCloseInfo&) {
        probe.enter("stopped");
        stopped.set();
    };

    TcpServer server{runtime, std::move(handlers)};
    REQUIRE(server.start(Endpoint{"127.0.0.1", 0}));
    REQUIRE(listening.wait());
    const auto endpoint = nativeEndpoint(listening.get());

    asio::io_context clientContext;
    tcp::socket first{clientContext};
    tcp::socket second{clientContext};
    first.connect(endpoint);
    second.connect(endpoint);
    REQUIRE(connected.waitFor(2));
    asio::write(first, asio::buffer(bytes("a")));
    asio::write(second, asio::buffer(bytes("b")));
    REQUIRE(dataReceived.waitFor(2));

    asio::error_code ignoredError;
    first.close(ignoredError);
    second.close(ignoredError);
    REQUIRE(connectionClosed.waitFor(2));
    server.stop();
    REQUIRE(stopped.wait());
    REQUIRE(probe.waitForSize(8));
    REQUIRE(probe.maxActive() == 1);

    const auto events = probe.events();
    REQUIRE(events.front() == "listening");
    REQUIRE(events.back() == "stopped");
    for (const auto& event : events)
    {
        if (!event.starts_with("data:"))
        {
            continue;
        }
        const std::string id = event.substr(5);
        const auto connectedPosition = std::find(events.begin(), events.end(), "connected:" + id);
        const auto dataPosition = std::find(events.begin(), events.end(), event);
        const auto closedPosition = std::find(events.begin(), events.end(), "closed:" + id);
        REQUIRE(connectedPosition < dataPosition);
        REQUIRE(dataPosition < closedPosition);
    }
    runtime.shutdown();
}

TEST_CASE("TcpServer emits all connection closes before stopped", "[NetworkTransportUtils][TcpServer]")
{
    NetworkRuntime runtime;
    OneShot<Endpoint> listening;
    EventCounter connected;
    OneShot<void> stopped;
    std::atomic<bool> closeOriginsWereLocal{true};
    std::atomic<bool> lifecycleStatesWereOrdered{true};
    std::mutex orderMutex;
    std::vector<std::string> order;
    TcpServer* serverAddress{nullptr};

    TcpServerHandlers handlers;
    handlers.onListening = [&listening](const Endpoint& endpoint) { listening.set(endpoint); };
    handlers.onConnected = [&connected](const TcpConnectionInfo&) { connected.signal(); };
    handlers.onConnectionClosed = [&orderMutex,
                                      &order,
                                      &closeOriginsWereLocal,
                                      &lifecycleStatesWereOrdered,
                                      &serverAddress](TcpConnectionId, const TransportCloseInfo& info) {
        if (info.origin != CloseOrigin::Local)
        {
            closeOriginsWereLocal.store(false);
        }
        if (serverAddress->state() != TcpServerState::Stopping)
        {
            lifecycleStatesWereOrdered.store(false);
        }
        std::scoped_lock lock{orderMutex};
        order.emplace_back("closed");
    };
    handlers.onStopped = [&orderMutex,
                             &order,
                             &stopped,
                             &closeOriginsWereLocal,
                             &lifecycleStatesWereOrdered,
                             &serverAddress](const TransportCloseInfo& info) {
        if (info.origin != CloseOrigin::Local)
        {
            closeOriginsWereLocal.store(false);
        }
        if (serverAddress->state() != TcpServerState::Idle)
        {
            lifecycleStatesWereOrdered.store(false);
        }
        {
            std::scoped_lock lock{orderMutex};
            order.emplace_back("stopped");
        }
        stopped.set();
    };

    TcpServer server{runtime, std::move(handlers)};
    serverAddress = &server;
    REQUIRE(server.start(Endpoint{"127.0.0.1", 0}));
    REQUIRE(listening.wait());
    const auto endpoint = nativeEndpoint(listening.get());
    asio::io_context ioContext;
    tcp::socket first{ioContext};
    tcp::socket second{ioContext};
    first.connect(endpoint);
    second.connect(endpoint);
    REQUIRE(connected.waitFor(2));

    server.stop();
    REQUIRE(stopped.wait());
    {
        std::scoped_lock lock{orderMutex};
        REQUIRE(order == std::vector<std::string>{"closed", "closed", "stopped"});
    }
    REQUIRE(closeOriginsWereLocal.load());
    REQUIRE(lifecycleStatesWereOrdered.load());
    REQUIRE(server.connectionCount() == 0);
    runtime.shutdown();
}

TEST_CASE("TcpServer enforces per-session and global write budgets", "[NetworkTransportUtils][TcpServer]")
{
    NetworkRuntime runtime;
    OneShot<Endpoint> listening;
    EventCounter connected;
    OneShot<TransportCloseInfo> stopped;
    std::mutex idsMutex;
    std::vector<TcpConnectionId> ids;

    TcpServerOptions options;
    options.maxQueuedWriteBytesPerConnection = 4;
    options.maxTotalQueuedWriteBytes = 5;
    TcpServerHandlers handlers;
    handlers.onListening = [&listening](const Endpoint& endpoint) { listening.set(endpoint); };
    handlers.onConnected = [&connected, &idsMutex, &ids](const TcpConnectionInfo& info) {
        {
            std::scoped_lock lock{idsMutex};
            ids.emplace_back(info.id);
        }
        connected.signal();
    };
    handlers.onStopped = [&stopped](const TransportCloseInfo& info) { stopped.set(info); };

    TcpServer server{runtime, options, std::move(handlers)};
    REQUIRE(server.start(Endpoint{"127.0.0.1", 0}));
    REQUIRE(listening.wait());
    const auto endpoint = nativeEndpoint(listening.get());
    asio::io_context ioContext;
    tcp::socket first{ioContext};
    tcp::socket second{ioContext};
    first.connect(endpoint);
    second.connect(endpoint);
    REQUIRE(connected.waitFor(2));

    OneShot<void> blockerEntered;
    std::promise<void> releaseBlocker;
    auto release = releaseBlocker.get_future().share();
    REQUIRE(NetworkRuntimeAccess::post(runtime, [&blockerEntered, release] {
        blockerEntered.set();
        release.wait();
    }));
    REQUIRE(blockerEntered.wait());

    std::vector<TcpConnectionId> acceptedIds;
    {
        std::scoped_lock lock{idsMutex};
        acceptedIds = ids;
    }
    REQUIRE(server.send(acceptedIds[0], bytes("1234")));
    const auto perConnectionFull = server.send(acceptedIds[0], bytes("5"));
    REQUIRE_FALSE(perConnectionFull);
    REQUIRE(perConnectionFull.error().code == NetworkErrorCode::QueueFull);
    const auto totalFull = server.send(acceptedIds[1], bytes("12"));
    REQUIRE_FALSE(totalFull);
    REQUIRE(totalFull.error().code == NetworkErrorCode::QueueFull);
    REQUIRE(server.send(acceptedIds[1], bytes("1")));

    server.stop();
    releaseBlocker.set_value();
    REQUIRE(stopped.wait());
    REQUIRE(stopped.get().discardedWriteBytes == 5);
    runtime.shutdown();
}

TEST_CASE("TcpServer turns a data callback exception into a connection terminal error", "[NetworkTransportUtils][TcpServer]")
{
    NetworkRuntime runtime;
    OneShot<Endpoint> listening;
    OneShot<TransportCloseInfo> closed;
    OneShot<void> stopped;
    TcpServerHandlers handlers;
    handlers.onListening = [&listening](const Endpoint& endpoint) { listening.set(endpoint); };
    handlers.onDataReceived = [](TcpConnectionId, ByteView) { throw std::runtime_error{"bad parser"}; };
    handlers.onConnectionClosed = [&closed](TcpConnectionId, const TransportCloseInfo& info) { closed.set(info); };
    handlers.onStopped = [&stopped](const TransportCloseInfo&) { stopped.set(); };

    TcpServer server{runtime, std::move(handlers)};
    REQUIRE(server.start(Endpoint{"127.0.0.1", 0}));
    REQUIRE(listening.wait());
    asio::io_context ioContext;
    tcp::socket client{ioContext};
    client.connect(nativeEndpoint(listening.get()));
    asio::write(client, asio::buffer(bytes("x")));
    REQUIRE(closed.wait());
    const auto info = closed.get();
    REQUIRE(info.origin == CloseOrigin::Error);
    REQUIRE(info.cause.has_value());
    REQUIRE(info.cause->code == NetworkErrorCode::CallbackFailed);
    server.stop();
    REQUIRE(stopped.wait());
    runtime.shutdown();
}

TEST_CASE("TcpServer reports asynchronous bind failure through onStopped", "[NetworkTransportUtils][TcpServer]")
{
    asio::io_context blockerContext;
    tcp::acceptor blocker{blockerContext, tcp::endpoint{asio::ip::address_v4::loopback(), 0}};
    const auto port = blocker.local_endpoint().port();

    NetworkRuntime runtime;
    OneShot<TransportCloseInfo> stopped;
    std::atomic<int> listeningCount{0};
    TcpServerHandlers handlers;
    handlers.onListening = [&listeningCount](const Endpoint&) { listeningCount.fetch_add(1); };
    handlers.onStopped = [&stopped](const TransportCloseInfo& info) { stopped.set(info); };
    TcpServer server{runtime, std::move(handlers)};

    REQUIRE(server.start(Endpoint{"127.0.0.1", port}));
    REQUIRE(stopped.wait());
    const auto info = stopped.get();
    REQUIRE(info.origin == CloseOrigin::Error);
    REQUIRE(info.cause.has_value());
    REQUIRE(info.cause->code == NetworkErrorCode::AddressInUse);
    REQUIRE(listeningCount.load() == 0);
    REQUIRE(server.state() == TcpServerState::Idle);
    runtime.shutdown();
}

TEST_CASE("TcpServer runtime shutdown closes sessions and drains terminal callbacks", "[NetworkTransportUtils][TcpServer][Runtime]")
{
    NetworkRuntime runtime;
    OneShot<Endpoint> listening;
    OneShot<void> connected;
    OneShot<TransportCloseInfo> connectionClosed;
    OneShot<TransportCloseInfo> stopped;
    std::mutex orderMutex;
    std::vector<std::string> order;
    TcpServerHandlers handlers;
    handlers.onListening = [&listening](const Endpoint& endpoint) { listening.set(endpoint); };
    handlers.onConnected = [&connected](const TcpConnectionInfo&) { connected.set(); };
    handlers.onConnectionClosed = [&connectionClosed, &orderMutex, &order](TcpConnectionId, const TransportCloseInfo& info) {
        {
            std::scoped_lock lock{orderMutex};
            order.emplace_back("connection");
        }
        connectionClosed.set(info);
    };
    handlers.onStopped = [&stopped, &orderMutex, &order](const TransportCloseInfo& info) {
        {
            std::scoped_lock lock{orderMutex};
            order.emplace_back("server");
        }
        stopped.set(info);
    };
    TcpServer server{runtime, std::move(handlers)};
    REQUIRE(server.start(Endpoint{"127.0.0.1", 0}));
    REQUIRE(listening.wait());
    asio::io_context ioContext;
    tcp::socket client{ioContext};
    client.connect(nativeEndpoint(listening.get()));
    REQUIRE(connected.wait());

    runtime.shutdown();
    REQUIRE(connectionClosed.wait());
    REQUIRE(stopped.wait());
    REQUIRE(connectionClosed.get().origin == CloseOrigin::RuntimeShutdown);
    REQUIRE(stopped.get().origin == CloseOrigin::RuntimeShutdown);
    {
        std::scoped_lock lock{orderMutex};
        REQUIRE(order == std::vector<std::string>{"connection", "server"});
    }
    REQUIRE(server.state() == TcpServerState::Idle);
}

TEST_CASE("TcpServer validates limits and inactive commands", "[NetworkTransportUtils][TcpServer]")
{
    NetworkRuntime runtime;
    TcpServerOptions options;
    options.maxConnections = 0;
    REQUIRE_THROWS_AS(TcpServer(runtime, options, {}), std::invalid_argument);
    options = {};
    options.maxTotalQueuedWriteBytes = 0;
    REQUIRE_THROWS_AS(TcpServer(runtime, options, {}), std::invalid_argument);
    options = {};
    options.listenBacklog = 0;
    REQUIRE_THROWS_AS(TcpServer(runtime, options, {}), std::invalid_argument);
    options = {};
    options.listenBacklog = static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1;
    REQUIRE_THROWS_AS(TcpServer(runtime, options, {}), std::invalid_argument);
    options = {};
    options.readBufferSize = 0;
    REQUIRE_THROWS_AS(TcpServer(runtime, options, {}), std::invalid_argument);
    options = {};
    options.readBufferSize = std::numeric_limits<std::size_t>::max();
    REQUIRE_THROWS_AS(TcpServer(runtime, options, {}), std::invalid_argument);
    options = {};
    options.maxQueuedWriteBytesPerConnection = 0;
    REQUIRE_THROWS_AS(TcpServer(runtime, options, {}), std::invalid_argument);

    TcpServer server{runtime};
    const auto empty = server.send(42, {});
    const auto unknown = server.send(42, bytes("x"));
    REQUIRE_FALSE(empty);
    REQUIRE(empty.error().code == NetworkErrorCode::InvalidArgument);
    REQUIRE_FALSE(unknown);
    REQUIRE(unknown.error().code == NetworkErrorCode::InvalidState);
    server.disconnect(42);
    server.stop();
    runtime.shutdown();
}

TEST_CASE("TcpServer can restart after its terminal callback", "[NetworkTransportUtils][TcpServer]")
{
    NetworkRuntime runtime;
    EventCounter listening;
    EventCounter stopped;
    std::mutex endpointsMutex;
    std::vector<Endpoint> endpoints;
    TcpServerHandlers handlers;
    handlers.onListening = [&listening, &endpointsMutex, &endpoints](const Endpoint& endpoint) {
        {
            std::scoped_lock lock{endpointsMutex};
            endpoints.emplace_back(endpoint);
        }
        listening.signal();
    };
    handlers.onStopped = [&stopped](const TransportCloseInfo&) { stopped.signal(); };
    TcpServer server{runtime, std::move(handlers)};

    REQUIRE(server.start(Endpoint{"127.0.0.1", 0}));
    REQUIRE(listening.waitFor(1));
    server.stop();
    REQUIRE(stopped.waitFor(1));
    REQUIRE(server.state() == TcpServerState::Idle);
    REQUIRE(server.start(Endpoint{"127.0.0.1", 0}));
    REQUIRE(listening.waitFor(2));
    server.stop();
    REQUIRE(stopped.waitFor(2));
    {
        std::scoped_lock lock{endpointsMutex};
        REQUIRE(endpoints.size() == 2);
        REQUIRE(endpoints[0].port != 0);
        REQUIRE(endpoints[1].port != 0);
    }

    // Keep the first control command queued while stop() claims the new
    // lifecycle. This exercises restart bookkeeping before startActor has had
    // a chance to run.
    OneShot<void> blockerEntered;
    std::promise<void> releaseBlocker;
    auto release = releaseBlocker.get_future().share();
    REQUIRE(NetworkRuntimeAccess::post(runtime, [&blockerEntered, release] {
        blockerEntered.set();
        release.wait();
    }));
    REQUIRE(blockerEntered.wait());
    REQUIRE(server.start(Endpoint{"127.0.0.1", 0}));
    server.stop();
    releaseBlocker.set_value();
    REQUIRE(stopped.waitFor(3));
    REQUIRE(server.state() == TcpServerState::Idle);
    runtime.shutdown();
}

TEST_CASE("TcpServer resumes accept after maxConnections capacity is released", "[NetworkTransportUtils][TcpServer]")
{
    NetworkRuntime runtime;
    OneShot<Endpoint> listening;
    EventCounter connected;
    EventCounter closed;
    OneShot<void> stopped;
    TcpServerOptions options;
    options.maxConnections = 1;
    TcpServerHandlers handlers;
    handlers.onListening = [&listening](const Endpoint& endpoint) { listening.set(endpoint); };
    handlers.onConnected = [&connected](const TcpConnectionInfo&) { connected.signal(); };
    handlers.onConnectionClosed = [&closed](TcpConnectionId, const TransportCloseInfo&) { closed.signal(); };
    handlers.onStopped = [&stopped](const TransportCloseInfo&) { stopped.set(); };
    TcpServer server{runtime, options, std::move(handlers)};
    REQUIRE(server.start(Endpoint{"127.0.0.1", 0}));
    REQUIRE(listening.wait());

    asio::io_context ioContext;
    tcp::socket first{ioContext};
    tcp::socket second{ioContext};
    const auto endpoint = nativeEndpoint(listening.get());
    first.connect(endpoint);
    REQUIRE(connected.waitFor(1));
    second.connect(endpoint);
    std::this_thread::sleep_for(50ms);
    REQUIRE(connected.count() == 1);
    REQUIRE(server.connectionCount() == 1);

    asio::error_code ignoredError;
    first.close(ignoredError);
    REQUIRE(closed.waitFor(1));
    REQUIRE(connected.waitFor(2));
    REQUIRE(server.connectionCount() == 1);
    second.close(ignoredError);
    REQUIRE(closed.waitFor(2));
    server.stop();
    REQUIRE(stopped.wait());
    runtime.shutdown();
}
