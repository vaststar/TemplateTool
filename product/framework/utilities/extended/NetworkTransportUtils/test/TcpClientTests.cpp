#include <catch2/catch_test_macros.hpp>

#include <ucf/utilities/NetworkTransportUtils/runtime/NetworkRuntime.h>
#include <ucf/utilities/NetworkTransportUtils/tcp/TcpClient.h>

#include "NetworkTestSupport.h"
#include "runtime/NetworkRuntimeAccess.h"

#include <asio/buffer.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/post.hpp>
#include <asio/write.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace std::chrono_literals;
using asio::ip::tcp;
using network_test::ByteCollector;
using network_test::EventCounter;
using network_test::OneShot;
using network_test::bytes;
using ucf::utilities::network::ByteBuffer;
using ucf::utilities::network::ByteView;
using ucf::utilities::network::CloseOrigin;
using ucf::utilities::network::Endpoint;
using ucf::utilities::network::NetworkErrorCode;
using ucf::utilities::network::NetworkOperation;
using ucf::utilities::network::NetworkRuntime;
using ucf::utilities::network::TcpClient;
using ucf::utilities::network::TcpClientHandlers;
using ucf::utilities::network::TcpClientOptions;
using ucf::utilities::network::TcpClientState;
using ucf::utilities::network::TransportCloseInfo;
using ucf::utilities::network::detail::NetworkRuntimeAccess;

class LoopbackPeer final
{
public:
    explicit LoopbackPeer(bool echo = false)
        : mAcceptor{mIoContext, tcp::endpoint{asio::ip::address_v4::loopback(), 0}}
        , mSocket{mIoContext}
        , mWorkGuard{asio::make_work_guard(mIoContext)}
        , mEcho{echo}
    {
        mAcceptor.async_accept(mSocket, [this](const asio::error_code& error) {
            mAccepted.set(!error);
            if (!error && mEcho)
            {
                startRead();
            }
        });
        mWorker = std::thread{[this] {
            mIoContext.run();
        }};
    }

    ~LoopbackPeer()
    {
        asio::post(mIoContext, [this] {
            asio::error_code ignoredError;
            mSocket.close(ignoredError);
            mAcceptor.close(ignoredError);
            mWorkGuard.reset();
        });
        if (mWorker.joinable())
        {
            mWorker.join();
        }
    }

    [[nodiscard]] std::uint16_t port() const
    {
        return mAcceptor.local_endpoint().port();
    }

    [[nodiscard]] bool waitAccepted()
    {
        return mAccepted.wait() && mAccepted.get();
    }

    void closePeer()
    {
        asio::post(mIoContext, [this] {
            asio::error_code ignoredError;
            mSocket.shutdown(tcp::socket::shutdown_both, ignoredError);
            mSocket.close(ignoredError);
        });
    }

private:
    void startRead()
    {
        mSocket.async_read_some(asio::buffer(mReadBuffer), [this](const asio::error_code& error, std::size_t byteCount) {
            if (error)
            {
                return;
            }
            auto data = std::make_shared<ByteBuffer>(mReadBuffer.begin(), mReadBuffer.begin() + byteCount);
            asio::async_write(mSocket, asio::buffer(*data), [this, data](const asio::error_code& writeError, std::size_t) {
                if (!writeError)
                {
                    startRead();
                }
            });
        });
    }

private:
    asio::io_context mIoContext;
    tcp::acceptor mAcceptor;
    tcp::socket mSocket;
    asio::executor_work_guard<asio::io_context::executor_type> mWorkGuard;
    std::array<std::uint8_t, 4096> mReadBuffer{};
    OneShot<bool> mAccepted;
    bool mEcho;
    std::thread mWorker;
};

[[nodiscard]] std::uint16_t closedLoopbackPort()
{
    asio::io_context ioContext;
    tcp::acceptor acceptor{ioContext, tcp::endpoint{asio::ip::address_v4::loopback(), 0}};
    const auto port = acceptor.local_endpoint().port();
    asio::error_code ignoredError;
    acceptor.close(ignoredError);
    return port;
}

} // namespace

TEST_CASE("TcpClient validates options and commands", "[NetworkTransportUtils][TcpClient]")
{
    NetworkRuntime runtime;

    TcpClientOptions options;
    options.readBufferSize = 0;
    REQUIRE_THROWS_AS(TcpClient(runtime, options, {}), std::invalid_argument);
    options = {};
    options.readBufferSize = std::numeric_limits<std::size_t>::max();
    REQUIRE_THROWS_AS(TcpClient(runtime, options, {}), std::invalid_argument);
    options = {};
    options.maxQueuedWriteBytes = 0;
    REQUIRE_THROWS_AS(TcpClient(runtime, options, {}), std::invalid_argument);
    options = {};
    options.connectTimeout = -1ms;
    REQUIRE_THROWS_AS(TcpClient(runtime, options, {}), std::invalid_argument);

    TcpClient client{runtime};
    const auto emptyHost = client.connect(Endpoint{"", 42});
    const auto emptyPort = client.connect(Endpoint{"localhost", 0});
    const auto emptyWrite = client.send({});
    const auto disconnectedWrite = client.send(ByteBuffer{1});
    REQUIRE_FALSE(emptyHost);
    REQUIRE(emptyHost.error().code == NetworkErrorCode::InvalidArgument);
    REQUIRE_FALSE(emptyPort);
    REQUIRE(emptyPort.error().code == NetworkErrorCode::InvalidArgument);
    REQUIRE_FALSE(emptyWrite);
    REQUIRE(emptyWrite.error().code == NetworkErrorCode::InvalidArgument);
    REQUIRE_FALSE(disconnectedWrite);
    REQUIRE(disconnectedWrite.error().code == NetworkErrorCode::NotConnected);

    runtime.shutdown();
}

TEST_CASE("TcpClient exchanges ordered bytes and reports one local close", "[NetworkTransportUtils][TcpClient]")
{
    LoopbackPeer peer{true};
    NetworkRuntime runtime;
    OneShot<void> connected;
    OneShot<TransportCloseInfo> closed;
    EventCounter closedCount;
    ByteCollector received{6};

    TcpClientHandlers handlers;
    handlers.onConnected = [&connected] { connected.set(); };
    handlers.onDataReceived = [&received](ByteView data) { received.append(data); };
    handlers.onClosed = [&closed, &closedCount](const TransportCloseInfo& info) {
        closedCount.signal();
        closed.set(info);
    };

    TcpClient client{runtime, std::move(handlers)};
    REQUIRE(client.connect(Endpoint{"127.0.0.1", peer.port()}));
    REQUIRE(connected.wait());
    REQUIRE(peer.waitAccepted());
    REQUIRE(client.state() == TcpClientState::Connected);
    REQUIRE_FALSE(client.connect(Endpoint{"127.0.0.1", peer.port()}));

    REQUIRE(client.send(bytes("one")));
    REQUIRE(client.send(bytes("two")));
    REQUIRE(received.wait());
    REQUIRE(received.snapshot() == bytes("onetwo"));

    client.close();
    client.close();
    REQUIRE(closed.wait());
    REQUIRE(closed.get().origin == CloseOrigin::Local);
    REQUIRE(closedCount.count() == 1);
    REQUIRE(client.state() == TcpClientState::Idle);
    runtime.shutdown();
}

TEST_CASE("TcpClient reports asynchronous connection refusal through its terminal event", "[NetworkTransportUtils][TcpClient]")
{
    NetworkRuntime runtime;
    OneShot<TransportCloseInfo> closed;
    std::atomic<int> connectedCount{0};
    TcpClientHandlers handlers;
    handlers.onConnected = [&connectedCount] { connectedCount.fetch_add(1); };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };

    TcpClient client{runtime, std::move(handlers)};
    REQUIRE(client.connect(Endpoint{"127.0.0.1", closedLoopbackPort()}));
    REQUIRE(closed.wait());
    const TransportCloseInfo info = closed.get();
    REQUIRE(info.origin == CloseOrigin::Error);
    REQUIRE(info.cause.has_value());
    REQUIRE(info.cause->code == NetworkErrorCode::ConnectionRefused);
    REQUIRE(info.cause->operation == NetworkOperation::Connect);
    REQUIRE(connectedCount.load() == 0);
    REQUIRE(client.state() == TcpClientState::Idle);
    runtime.shutdown();
}

TEST_CASE("TcpClient distinguishes peer EOF from local close", "[NetworkTransportUtils][TcpClient]")
{
    LoopbackPeer peer;
    NetworkRuntime runtime;
    OneShot<void> connected;
    OneShot<TransportCloseInfo> closed;
    TcpClientHandlers handlers;
    handlers.onConnected = [&connected] { connected.set(); };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };

    TcpClient client{runtime, std::move(handlers)};
    REQUIRE(client.connect(Endpoint{"127.0.0.1", peer.port()}));
    REQUIRE(connected.wait());
    REQUIRE(peer.waitAccepted());
    peer.closePeer();
    REQUIRE(closed.wait());
    const TransportCloseInfo info = closed.get();
    REQUIRE(info.origin == CloseOrigin::Peer);
    REQUIRE(info.cause.has_value());
    REQUIRE((info.cause->code == NetworkErrorCode::EndOfStream
        || info.cause->code == NetworkErrorCode::ConnectionReset));
    runtime.shutdown();
}

TEST_CASE("TcpClient converts callback exceptions into a terminal error", "[NetworkTransportUtils][TcpClient]")
{
    LoopbackPeer peer;
    NetworkRuntime runtime;
    OneShot<TransportCloseInfo> closed;
    TcpClientHandlers handlers;
    handlers.onConnected = [] { throw std::runtime_error{"consumer failure"}; };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };

    TcpClient client{runtime, std::move(handlers)};
    REQUIRE(client.connect(Endpoint{"127.0.0.1", peer.port()}));
    REQUIRE(peer.waitAccepted());
    REQUIRE(closed.wait());
    const auto info = closed.get();
    REQUIRE(info.origin == CloseOrigin::Error);
    REQUIRE(info.cause.has_value());
    REQUIRE(info.cause->code == NetworkErrorCode::CallbackFailed);
    REQUIRE(info.cause->operation == NetworkOperation::Callback);
    runtime.shutdown();
}

TEST_CASE(
    "TcpClient preserves the first close reason when close races a callback failure",
    "[NetworkTransportUtils][TcpClient][Concurrency]")
{
    LoopbackPeer peer;
    NetworkRuntime runtime;
    OneShot<void> callbackEntered;
    OneShot<TransportCloseInfo> closed;
    std::promise<void> releaseCallback;
    const auto release = releaseCallback.get_future().share();

    TcpClientHandlers handlers;
    handlers.onConnected = [&callbackEntered, release] {
        callbackEntered.set();
        release.wait();
        throw std::runtime_error{"failure after local close"};
    };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };
    TcpClient client{runtime, std::move(handlers)};

    REQUIRE(client.connect(Endpoint{"127.0.0.1", peer.port()}));
    const bool entered = callbackEntered.wait();
    if (entered)
    {
        client.close();
    }
    releaseCallback.set_value();

    REQUIRE(entered);
    REQUIRE(peer.waitAccepted());
    REQUIRE(closed.wait());
    REQUIRE(closed.get().origin == CloseOrigin::Local);
    runtime.shutdown();
}

TEST_CASE("TcpClient concurrent close remains idempotent", "[NetworkTransportUtils][TcpClient][Concurrency]")
{
    LoopbackPeer peer;
    NetworkRuntime runtime;
    OneShot<void> connected;
    EventCounter closed;
    TcpClientHandlers handlers;
    handlers.onConnected = [&connected] { connected.set(); };
    handlers.onClosed = [&closed](const TransportCloseInfo&) { closed.signal(); };
    TcpClient client{runtime, std::move(handlers)};

    REQUIRE(client.connect(Endpoint{"127.0.0.1", peer.port()}));
    REQUIRE(connected.wait());
    REQUIRE(peer.waitAccepted());
    std::vector<std::thread> callers;
    for (int index = 0; index < 8; ++index)
    {
        callers.emplace_back([&client] { client.close(); });
    }
    for (auto& caller : callers)
    {
        caller.join();
    }
    REQUIRE(closed.waitFor(1));
    std::this_thread::sleep_for(50ms);
    REQUIRE(closed.count() == 1);
    runtime.shutdown();
}

TEST_CASE("TcpClient can close itself from onConnected", "[NetworkTransportUtils][TcpClient][Concurrency]")
{
    LoopbackPeer peer;
    NetworkRuntime runtime;
    OneShot<TransportCloseInfo> closed;
    TcpClient* clientAddress{nullptr};
    TcpClientHandlers handlers;
    handlers.onConnected = [&clientAddress] { clientAddress->close(); };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };
    TcpClient client{runtime, std::move(handlers)};
    clientAddress = &client;

    REQUIRE(client.connect(Endpoint{"127.0.0.1", peer.port()}));
    REQUIRE(peer.waitAccepted());
    REQUIRE(closed.wait());
    REQUIRE(closed.get().origin == CloseOrigin::Local);
    runtime.shutdown();
}

TEST_CASE("TcpClient enforces write admission before actor execution", "[NetworkTransportUtils][TcpClient]")
{
    LoopbackPeer peer;
    NetworkRuntime runtime;
    OneShot<void> connected;
    OneShot<TransportCloseInfo> closed;
    TcpClientOptions options;
    options.maxQueuedWriteBytes = 3;
    TcpClientHandlers handlers;
    handlers.onConnected = [&connected] { connected.set(); };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };
    TcpClient client{runtime, options, std::move(handlers)};
    REQUIRE(client.connect(Endpoint{"127.0.0.1", peer.port()}));
    REQUIRE(connected.wait());
    REQUIRE(peer.waitAccepted());

    OneShot<void> blockerEntered;
    std::promise<void> releaseBlocker;
    auto release = releaseBlocker.get_future().share();
    REQUIRE(NetworkRuntimeAccess::post(runtime, [&blockerEntered, release] {
        blockerEntered.set();
        release.wait();
    }));
    REQUIRE(blockerEntered.wait());

    REQUIRE(client.send(bytes("123")));
    const auto queueFull = client.send(bytes("4"));
    REQUIRE_FALSE(queueFull);
    REQUIRE(queueFull.error().code == NetworkErrorCode::QueueFull);
    client.close();
    releaseBlocker.set_value();
    REQUIRE(closed.wait());
    REQUIRE(closed.get().discardedWriteBytes == 3);
    runtime.shutdown();
}

TEST_CASE("TcpClient survives destruction of the NetworkRuntime facade and drains shutdown", "[NetworkTransportUtils][TcpClient][Runtime]")
{
    LoopbackPeer peer;
    auto runtime = std::make_unique<NetworkRuntime>();
    OneShot<void> connected;
    OneShot<TransportCloseInfo> closed;
    TcpClientHandlers handlers;
    handlers.onConnected = [&connected] { connected.set(); };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };
    auto client = std::make_unique<TcpClient>(*runtime, std::move(handlers));

    REQUIRE(client->connect(Endpoint{"127.0.0.1", peer.port()}));
    REQUIRE(connected.wait());
    REQUIRE(peer.waitAccepted());
    runtime.reset();
    REQUIRE(closed.wait());
    REQUIRE(closed.get().origin == CloseOrigin::RuntimeShutdown);
    REQUIRE(client->state() == TcpClientState::Idle);
    client.reset();
}

TEST_CASE("TcpClient is reusable after an asynchronous failure", "[NetworkTransportUtils][TcpClient]")
{
    LoopbackPeer peer;
    NetworkRuntime runtime;
    EventCounter connected;
    EventCounter closed;
    std::mutex infoMutex;
    std::vector<TransportCloseInfo> closeInfos;
    TcpClientHandlers handlers;
    handlers.onConnected = [&connected] { connected.signal(); };
    handlers.onClosed = [&closed, &infoMutex, &closeInfos](const TransportCloseInfo& info) {
        {
            std::scoped_lock lock{infoMutex};
            closeInfos.emplace_back(info);
        }
        closed.signal();
    };
    TcpClient client{runtime, std::move(handlers)};

    REQUIRE(client.connect(Endpoint{"127.0.0.1", closedLoopbackPort()}));
    REQUIRE(closed.waitFor(1));
    REQUIRE(client.state() == TcpClientState::Idle);
    REQUIRE(client.connect(Endpoint{"127.0.0.1", peer.port()}));
    REQUIRE(connected.waitFor(1));
    REQUIRE(peer.waitAccepted());
    client.close();
    REQUIRE(closed.waitFor(2));
    {
        std::scoped_lock lock{infoMutex};
        REQUIRE(closeInfos.size() == 2);
        REQUIRE(closeInfos[0].origin == CloseOrigin::Error);
        REQUIRE(closeInfos[1].origin == CloseOrigin::Local);
    }
    runtime.shutdown();
}

TEST_CASE("TcpClient reconnects from onClosed while a canceled read handler is pending", "[NetworkTransportUtils][TcpClient][Concurrency]")
{
    LoopbackPeer firstPeer;
    LoopbackPeer secondPeer;
    NetworkRuntime runtime;
    EventCounter connected;
    EventCounter closed;
    OneShot<void> firstReadStarted;
    std::atomic<bool> markerPostAccepted{false};
    std::atomic<bool> reconnectAccepted{false};
    TcpClient* clientAddress{nullptr};

    TcpClientHandlers handlers;
    handlers.onConnected = [&runtime, &connected, &firstReadStarted, &markerPostAccepted] {
        connected.signal();
        if (connected.count() == 1)
        {
            markerPostAccepted.store(NetworkRuntimeAccess::post(runtime, [&firstReadStarted] {
                // With one runtime worker this runs after handleConnect has
                // returned from onConnected and initiated its first read.
                firstReadStarted.set();
            }));
        }
    };
    handlers.onClosed = [&closed, &clientAddress, &secondPeer, &reconnectAccepted](const TransportCloseInfo&) {
        if (closed.count() == 0)
        {
            reconnectAccepted.store(static_cast<bool>(
                clientAddress->connect(Endpoint{"127.0.0.1", secondPeer.port()})));
        }
        closed.signal();
    };
    TcpClient client{runtime, std::move(handlers)};
    clientAddress = &client;

    REQUIRE(client.connect(Endpoint{"127.0.0.1", firstPeer.port()}));
    REQUIRE(firstPeer.waitAccepted());
    REQUIRE(firstReadStarted.wait());
    REQUIRE(markerPostAccepted.load());
    client.close();

    REQUIRE(closed.waitFor(1));
    REQUIRE(reconnectAccepted.load());
    REQUIRE(connected.waitFor(2));
    REQUIRE(secondPeer.waitAccepted());
    client.close();
    REQUIRE(closed.waitFor(2));
    runtime.shutdown();
}
