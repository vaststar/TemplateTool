#include <catch2/catch_test_macros.hpp>

#include <ucf/utilities/NetworkTransportUtils/runtime/NetworkRuntime.h>
#include <ucf/utilities/NetworkTransportUtils/udp/UdpSocket.h>

#include "NetworkTestSupport.h"
#include "runtime/NetworkRuntimeAccess.h"

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <future>
#include <mutex>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using namespace std::chrono_literals;
using asio::ip::udp;
using network_test::EventCounter;
using network_test::OneShot;
using network_test::bytes;
using ucf::utilities::network::ByteBuffer;
using ucf::utilities::network::ByteView;
using ucf::utilities::network::CloseOrigin;
using ucf::utilities::network::Endpoint;
using ucf::utilities::network::NetworkError;
using ucf::utilities::network::NetworkErrorCode;
using ucf::utilities::network::NetworkOperation;
using ucf::utilities::network::NetworkRuntime;
using ucf::utilities::network::NetworkRuntimeOptions;
using ucf::utilities::network::TransportCloseInfo;
using ucf::utilities::network::UdpSocket;
using ucf::utilities::network::UdpSocketHandlers;
using ucf::utilities::network::UdpSocketOptions;
using ucf::utilities::network::UdpSocketState;
using ucf::utilities::network::detail::NetworkRuntimeAccess;

struct Datagram final
{
    Endpoint sender;
    ByteBuffer data;
};

class DatagramCollector final
{
public:
    void append(const Endpoint& sender, ByteView data)
    {
        {
            std::scoped_lock lock{mMutex};
            mDatagrams.emplace_back(Datagram{sender, ByteBuffer{data.begin(), data.end()}});
        }
        mCondition.notify_all();
    }

    [[nodiscard]] bool waitFor(std::size_t count)
    {
        std::unique_lock lock{mMutex};
        return mCondition.wait_for(lock, 3s, [this, count] { return mDatagrams.size() >= count; });
    }

    [[nodiscard]] std::vector<Datagram> snapshot() const
    {
        std::scoped_lock lock{mMutex};
        return mDatagrams;
    }

private:
    mutable std::mutex mMutex;
    std::condition_variable mCondition;
    std::vector<Datagram> mDatagrams;
};

[[nodiscard]] udp::endpoint nativeEndpoint(const Endpoint& endpoint)
{
    return udp::endpoint{asio::ip::make_address(endpoint.host), endpoint.port};
}

} // namespace

TEST_CASE("UdpSocket binds asynchronously and exchanges ordered complete datagrams", "[NetworkTransportUtils][UdpSocket]")
{
    NetworkRuntimeOptions runtimeOptions;
    runtimeOptions.ioThreadCount = 2;
    NetworkRuntime runtime{runtimeOptions};
    OneShot<Endpoint> receiverBound;
    OneShot<Endpoint> senderBound;
    OneShot<TransportCloseInfo> receiverClosed;
    OneShot<TransportCloseInfo> senderClosed;
    DatagramCollector received;
    DatagramCollector echoed;
    std::atomic<int> errors{0};
    UdpSocket* receiverAddress{nullptr};

    UdpSocketHandlers receiverHandlers;
    receiverHandlers.onBound = [&receiverBound](const Endpoint& endpoint) { receiverBound.set(endpoint); };
    receiverHandlers.onDatagramReceived = [&received, &receiverAddress](const Endpoint& sender, ByteView data) {
        received.append(sender, data);
        const auto result = receiverAddress->sendTo(sender, ByteBuffer{data.begin(), data.end()});
        if (!result)
        {
            throw std::runtime_error{"UDP echo send was rejected"};
        }
    };
    receiverHandlers.onSendError = [&errors](const Endpoint&, const NetworkError&) { errors.fetch_add(1); };
    receiverHandlers.onReceiveError = [&errors](const NetworkError&) { errors.fetch_add(1); };
    receiverHandlers.onClosed = [&receiverClosed](const TransportCloseInfo& info) { receiverClosed.set(info); };
    UdpSocket receiver{runtime, std::move(receiverHandlers)};
    receiverAddress = &receiver;

    UdpSocketHandlers senderHandlers;
    senderHandlers.onBound = [&senderBound](const Endpoint& endpoint) { senderBound.set(endpoint); };
    senderHandlers.onDatagramReceived = [&echoed](const Endpoint& sender, ByteView data) { echoed.append(sender, data); };
    senderHandlers.onSendError = [&errors](const Endpoint&, const NetworkError&) { errors.fetch_add(1); };
    senderHandlers.onReceiveError = [&errors](const NetworkError&) { errors.fetch_add(1); };
    senderHandlers.onClosed = [&senderClosed](const TransportCloseInfo& info) { senderClosed.set(info); };
    UdpSocket sender{runtime, std::move(senderHandlers)};

    REQUIRE(receiver.bind(Endpoint{"127.0.0.1", 0}));
    REQUIRE(sender.bind(Endpoint{"127.0.0.1", 0}));
    REQUIRE(receiverBound.wait());
    REQUIRE(senderBound.wait());
    const Endpoint receiverEndpoint = receiverBound.get();
    const Endpoint senderEndpoint = senderBound.get();
    const Endpoint destination{"localhost", receiverEndpoint.port};
    REQUIRE(sender.sendTo(destination, bytes("one")));
    REQUIRE(sender.sendTo(destination, {}));
    REQUIRE(sender.sendTo(destination, bytes("three")));
    REQUIRE(received.waitFor(3));
    REQUIRE(echoed.waitFor(3));

    const auto receivedDatagrams = received.snapshot();
    const auto echoedDatagrams = echoed.snapshot();
    REQUIRE(receivedDatagrams[0].data == bytes("one"));
    REQUIRE(receivedDatagrams[1].data.empty());
    REQUIRE(receivedDatagrams[2].data == bytes("three"));
    REQUIRE(echoedDatagrams[0].data == receivedDatagrams[0].data);
    REQUIRE(echoedDatagrams[1].data == receivedDatagrams[1].data);
    REQUIRE(echoedDatagrams[2].data == receivedDatagrams[2].data);
    REQUIRE(receivedDatagrams[0].sender == senderEndpoint);
    REQUIRE(echoedDatagrams[0].sender == receiverEndpoint);
    REQUIRE(errors.load() == 0);

    sender.close();
    receiver.close();
    REQUIRE(senderClosed.wait());
    REQUIRE(receiverClosed.wait());
    REQUIRE(senderClosed.get().origin == CloseOrigin::Local);
    REQUIRE(receiverClosed.get().origin == CloseOrigin::Local);
    runtime.shutdown();
}

TEST_CASE("UdpSocket validates options and commands", "[NetworkTransportUtils][UdpSocket]")
{
    NetworkRuntime runtime;
    UdpSocketOptions options;
    options.maxDatagramSize = 0;
    REQUIRE_THROWS_AS(UdpSocket(runtime, options, {}), std::invalid_argument);
    options = {};
    options.maxDatagramSize = 65'508;
    REQUIRE_THROWS_AS(UdpSocket(runtime, options, {}), std::invalid_argument);
    options = {};
    options.maxQueuedDatagrams = 0;
    REQUIRE_THROWS_AS(UdpSocket(runtime, options, {}), std::invalid_argument);
    options = {};
    options.maxQueuedSendBytes = 0;
    REQUIRE_THROWS_AS(UdpSocket(runtime, options, {}), std::invalid_argument);
    options = {};
    options.dnsCacheTtl = -1ms;
    REQUIRE_THROWS_AS(UdpSocket(runtime, options, {}), std::invalid_argument);

    UdpSocket socket{runtime};
    const auto invalidHost = socket.sendTo(Endpoint{"", 42}, {});
    const auto invalidPort = socket.sendTo(Endpoint{"127.0.0.1", 0}, {});
    const auto notBound = socket.sendTo(Endpoint{"127.0.0.1", 42}, {});
    REQUIRE_FALSE(invalidHost);
    REQUIRE(invalidHost.error().code == NetworkErrorCode::InvalidArgument);
    REQUIRE_FALSE(invalidPort);
    REQUIRE(invalidPort.error().code == NetworkErrorCode::InvalidArgument);
    REQUIRE_FALSE(notBound);
    REQUIRE(notBound.error().code == NetworkErrorCode::InvalidState);
    runtime.shutdown();
}

TEST_CASE("UdpSocket bounds both bytes and zero-length datagram count", "[NetworkTransportUtils][UdpSocket]")
{
    NetworkRuntime runtime;
    OneShot<Endpoint> bound;
    OneShot<TransportCloseInfo> closed;
    UdpSocketOptions options;
    options.maxQueuedSendBytes = 3;
    options.maxQueuedDatagrams = 2;
    UdpSocketHandlers handlers;
    handlers.onBound = [&bound](const Endpoint& endpoint) { bound.set(endpoint); };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };
    UdpSocket socket{runtime, options, std::move(handlers)};
    REQUIRE(socket.bind(Endpoint{"127.0.0.1", 0}));
    REQUIRE(bound.wait());

    OneShot<void> blockerEntered;
    std::promise<void> releaseBlocker;
    auto release = releaseBlocker.get_future().share();
    REQUIRE(NetworkRuntimeAccess::post(runtime, [&blockerEntered, release] {
        blockerEntered.set();
        release.wait();
    }));
    REQUIRE(blockerEntered.wait());

    const Endpoint destination{"127.0.0.1", 9};
    REQUIRE(socket.sendTo(destination, bytes("123")));
    REQUIRE(socket.sendTo(destination, {}));
    const auto datagramFull = socket.sendTo(destination, {});
    REQUIRE_FALSE(datagramFull);
    REQUIRE(datagramFull.error().code == NetworkErrorCode::QueueFull);
    const auto byteFull = socket.sendTo(destination, bytes("1"));
    REQUIRE_FALSE(byteFull);
    REQUIRE(byteFull.error().code == NetworkErrorCode::QueueFull);

    socket.close();
    releaseBlocker.set_value();
    REQUIRE(closed.wait());
    REQUIRE(closed.get().discardedWriteBytes == 3);
    runtime.shutdown();
}

TEST_CASE("UdpSocket continues after an incompatible destination", "[NetworkTransportUtils][UdpSocket]")
{
    NetworkRuntime runtime;
    OneShot<Endpoint> receiverBound;
    OneShot<Endpoint> senderBound;
    OneShot<std::pair<Endpoint, NetworkError>> sendError;
    DatagramCollector received;
    OneShot<void> receiverClosed;
    OneShot<void> senderClosed;

    UdpSocketHandlers receiverHandlers;
    receiverHandlers.onBound = [&receiverBound](const Endpoint& endpoint) { receiverBound.set(endpoint); };
    receiverHandlers.onDatagramReceived = [&received](const Endpoint& sender, ByteView data) { received.append(sender, data); };
    receiverHandlers.onClosed = [&receiverClosed](const TransportCloseInfo&) { receiverClosed.set(); };
    UdpSocket receiver{runtime, std::move(receiverHandlers)};
    REQUIRE(receiver.bind(Endpoint{"127.0.0.1", 0}));

    UdpSocketHandlers senderHandlers;
    senderHandlers.onBound = [&senderBound](const Endpoint& endpoint) { senderBound.set(endpoint); };
    senderHandlers.onSendError = [&sendError](const Endpoint& endpoint, const NetworkError& error) {
        sendError.set(std::make_pair(endpoint, error));
    };
    senderHandlers.onClosed = [&senderClosed](const TransportCloseInfo&) { senderClosed.set(); };
    UdpSocket sender{runtime, std::move(senderHandlers)};
    REQUIRE(sender.bind(Endpoint{"127.0.0.1", 0}));
    REQUIRE(receiverBound.wait());
    REQUIRE(senderBound.wait());

    const Endpoint incompatible{"::1", receiverBound.get().port};
    REQUIRE(sender.sendTo(incompatible, bytes("bad")));
    REQUIRE(sender.sendTo(Endpoint{"127.0.0.1", incompatible.port}, bytes("good")));
    REQUIRE(sendError.wait());
    const auto [destination, error] = sendError.get();
    REQUIRE(destination == incompatible);
    REQUIRE(error.code == NetworkErrorCode::AddressNotAvailable);
    REQUIRE(error.operation == NetworkOperation::Resolve);
    REQUIRE(received.waitFor(1));
    REQUIRE(received.snapshot()[0].data == bytes("good"));
    REQUIRE(sender.state() == UdpSocketState::Bound);

    sender.close();
    receiver.close();
    REQUIRE(senderClosed.wait());
    REQUIRE(receiverClosed.wait());
    runtime.shutdown();
}

TEST_CASE("UdpSocket reports and retries an oversized receive", "[NetworkTransportUtils][UdpSocket]")
{
    NetworkRuntime runtime;
    OneShot<Endpoint> bound;
    OneShot<NetworkError> receiveError;
    OneShot<void> closed;
    std::atomic<int> receivedCount{0};
    UdpSocket* socketAddress{nullptr};
    UdpSocketOptions options;
    options.maxDatagramSize = 4;
    UdpSocketHandlers handlers;
    handlers.onBound = [&bound](const Endpoint& endpoint) { bound.set(endpoint); };
    handlers.onDatagramReceived = [&receivedCount](const Endpoint&, ByteView) { receivedCount.fetch_add(1); };
    handlers.onReceiveError = [&receiveError, &socketAddress](const NetworkError& error) {
        receiveError.set(error);
        socketAddress->close();
    };
    handlers.onClosed = [&closed](const TransportCloseInfo&) { closed.set(); };
    UdpSocket receiver{runtime, options, std::move(handlers)};
    socketAddress = &receiver;
    REQUIRE(receiver.bind(Endpoint{"127.0.0.1", 0}));
    REQUIRE(bound.wait());

    asio::io_context ioContext;
    udp::socket sender{ioContext, udp::v4()};
    const ByteBuffer oversized = bytes("12345");
    REQUIRE(sender.send_to(asio::buffer(oversized), nativeEndpoint(bound.get())) == oversized.size());
    REQUIRE(receiveError.wait());
    const auto error = receiveError.get();
    REQUIRE(error.code == NetworkErrorCode::MessageTooLarge);
    REQUIRE(error.operation == NetworkOperation::Receive);
    REQUIRE(receivedCount.load() == 0);
    REQUIRE(closed.wait());
    runtime.shutdown();
}

TEST_CASE("UdpSocket converts a datagram callback exception into terminal close", "[NetworkTransportUtils][UdpSocket]")
{
    NetworkRuntime runtime;
    OneShot<Endpoint> bound;
    OneShot<TransportCloseInfo> closed;
    UdpSocketHandlers handlers;
    handlers.onBound = [&bound](const Endpoint& endpoint) { bound.set(endpoint); };
    handlers.onDatagramReceived = [](const Endpoint&, ByteView) { throw std::runtime_error{"consumer failure"}; };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };
    UdpSocket receiver{runtime, std::move(handlers)};
    REQUIRE(receiver.bind(Endpoint{"127.0.0.1", 0}));
    REQUIRE(bound.wait());

    asio::io_context ioContext;
    udp::socket sender{ioContext, udp::v4()};
    const ByteBuffer data = bytes("x");
    sender.send_to(asio::buffer(data), nativeEndpoint(bound.get()));
    REQUIRE(closed.wait());
    const auto info = closed.get();
    REQUIRE(info.origin == CloseOrigin::Error);
    REQUIRE(info.cause.has_value());
    REQUIRE(info.cause->code == NetworkErrorCode::CallbackFailed);
    runtime.shutdown();
}

TEST_CASE("UdpSocket reports asynchronous bind failure", "[NetworkTransportUtils][UdpSocket]")
{
    asio::io_context blockerContext;
    udp::socket blocker{blockerContext, udp::endpoint{udp::v4(), 0}};
    const auto port = blocker.local_endpoint().port();
    NetworkRuntime runtime;
    OneShot<TransportCloseInfo> closed;
    std::atomic<int> boundCount{0};
    UdpSocketHandlers handlers;
    handlers.onBound = [&boundCount](const Endpoint&) { boundCount.fetch_add(1); };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };
    UdpSocket socket{runtime, std::move(handlers)};
    REQUIRE(socket.bind(Endpoint{"127.0.0.1", port}));
    REQUIRE(closed.wait());
    const auto info = closed.get();
    REQUIRE(info.origin == CloseOrigin::Error);
    REQUIRE(info.cause.has_value());
    REQUIRE(info.cause->code == NetworkErrorCode::AddressInUse);
    REQUIRE(boundCount.load() == 0);
    runtime.shutdown();
}

TEST_CASE("UdpSocket runtime shutdown drains onClosed", "[NetworkTransportUtils][UdpSocket][Runtime]")
{
    NetworkRuntime runtime;
    OneShot<Endpoint> bound;
    OneShot<TransportCloseInfo> closed;
    UdpSocketHandlers handlers;
    handlers.onBound = [&bound](const Endpoint& endpoint) { bound.set(endpoint); };
    handlers.onClosed = [&closed](const TransportCloseInfo& info) { closed.set(info); };
    UdpSocket socket{runtime, std::move(handlers)};
    REQUIRE(socket.bind(Endpoint{"127.0.0.1", 0}));
    REQUIRE(bound.wait());
    runtime.shutdown();
    REQUIRE(closed.wait());
    REQUIRE(closed.get().origin == CloseOrigin::RuntimeShutdown);
    REQUIRE(socket.state() == UdpSocketState::Idle);
}

TEST_CASE("UdpSocket can bind again after close", "[NetworkTransportUtils][UdpSocket]")
{
    NetworkRuntime runtime;
    EventCounter bound;
    EventCounter closed;
    DatagramCollector received;
    std::mutex endpointsMutex;
    std::vector<Endpoint> endpoints;
    std::atomic<bool> rebindAccepted{false};
    UdpSocket* socketAddress{nullptr};
    UdpSocketHandlers handlers;
    handlers.onBound = [&bound, &endpointsMutex, &endpoints](const Endpoint& endpoint) {
        {
            std::scoped_lock lock{endpointsMutex};
            endpoints.emplace_back(endpoint);
        }
        bound.signal();
    };
    handlers.onDatagramReceived = [&received](const Endpoint& sender, ByteView data) {
        received.append(sender, data);
    };
    handlers.onClosed = [&closed, &rebindAccepted, &socketAddress](const TransportCloseInfo&) {
        if (closed.count() == 0)
        {
            rebindAccepted.store(static_cast<bool>(
                socketAddress->bind(Endpoint{"127.0.0.1", 0})));
        }
        closed.signal();
    };
    UdpSocket socket{runtime, std::move(handlers)};
    socketAddress = &socket;

    REQUIRE(socket.bind(Endpoint{"127.0.0.1", 0}));
    REQUIRE(bound.waitFor(1));
    socket.close();
    REQUIRE(closed.waitFor(1));
    REQUIRE(rebindAccepted.load());
    REQUIRE(bound.waitFor(2));

    Endpoint reboundEndpoint;
    {
        std::scoped_lock lock{endpointsMutex};
        reboundEndpoint = endpoints[1];
    }
    asio::io_context senderContext;
    udp::socket sender{senderContext, udp::endpoint{udp::v4(), 0}};
    const ByteBuffer payload = bytes("after-rebind");
    REQUIRE(sender.send_to(asio::buffer(payload), nativeEndpoint(reboundEndpoint)) == payload.size());
    REQUIRE(received.waitFor(1));
    REQUIRE(received.snapshot()[0].data == payload);

    socket.close();
    REQUIRE(closed.waitFor(2));
    {
        std::scoped_lock lock{endpointsMutex};
        REQUIRE(endpoints.size() == 2);
        REQUIRE(endpoints[0].port != 0);
        REQUIRE(endpoints[1].port != 0);
    }
    runtime.shutdown();
}
