#include <ucf/utilities/NetworkTransportUtils/tcp/TcpClient.h>

#include "NetworkTransportLogger.h"
#include "core/AsioErrorMapper.h"
#include "core/CallbackUtils.h"
#include "core/ExceptionErrorMapper.h"
#include "core/NetworkEnumStrings.h"
#include "core/NetworkErrorUtils.h"
#include "core/SteadyClockUtils.h"
#include "runtime/NetworkRuntimeAccess.h"
#include "runtime/RuntimeCore.h"
#include "tcp/TcpCloseInfoUtils.h"

#include <asio/buffer.hpp>
#include <asio/connect.hpp>
#include <asio/error.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/steady_timer.hpp>
#include <asio/strand.hpp>
#include <asio/write.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace ucf::utilities::network {

namespace {

using ConnectDeadline = std::optional<std::chrono::steady_clock::time_point>;
using asio::ip::tcp;

[[nodiscard]] TcpClientOptions validateOptions(TcpClientOptions options)
{
    if (options.readBufferSize == 0
        || options.readBufferSize > ByteBuffer{}.max_size()
        || options.maxQueuedWriteBytes == 0)
    {
        throw std::invalid_argument{"TcpClient buffer sizes are invalid"};
    }
    if (options.connectTimeout < std::chrono::milliseconds::zero())
    {
        throw std::invalid_argument{"TcpClient connectTimeout must not be negative"};
    }
    return options;
}

} // namespace

class TcpClient::Impl final
    : public detail::RuntimeParticipant
    , public std::enable_shared_from_this<TcpClient::Impl>
{
public:
    Impl(NetworkRuntime& runtime, TcpClientOptions options, TcpClientHandlers handlers)
        : mCore{detail::NetworkRuntimeAccess::core(runtime)}
        , mStrand{asio::make_strand(mCore->executor())}
        , mResolver{mStrand}
        , mSocket{mStrand}
        , mConnectTimer{mStrand}
        , mOptions{validateOptions(options)}
        , mHandlers{std::move(handlers)}
    {
    }

    [[nodiscard]] NetworkResult<void> connect(Endpoint endpoint)
    {
        if (endpoint.host.empty())
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidArgument,
                NetworkOperation::Connect,
                "TCP client host must not be empty"));
        }
        if (endpoint.port == 0)
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidArgument,
                NetworkOperation::Connect,
                "TCP client port must be greater than zero"));
        }

        std::scoped_lock lock{mCommandMutex};
        if (!mCore->isRunning())
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Connect,
                "NetworkRuntime is not accepting new work"));
        }
        if (mState.load(std::memory_order_acquire) != TcpClientState::Idle)
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Connect,
                "TCP client already has an active lifecycle"));
        }

        const std::uint64_t generation = mGeneration.fetch_add(1, std::memory_order_acq_rel) + 1;
        mState.store(TcpClientState::Connecting, std::memory_order_release);
        ConnectDeadline deadline;
        if (mOptions.connectTimeout > std::chrono::milliseconds::zero())
        {
            deadline = detail::steadyDeadlineAfter(mOptions.connectTimeout);
        }

        try
        {
            const auto activityId = mCore->startActivity(
                shared_from_this(),
                mStrand,
                [self = shared_from_this(), endpoint = std::move(endpoint), deadline, generation](detail::RuntimeActivityId id) mutable {
                    self->startConnect(std::move(endpoint), deadline, generation, id);
                });
            if (!activityId)
            {
                mState.store(TcpClientState::Idle, std::memory_order_release);
                return NetworkResult<void>::failure(detail::makeError(
                    NetworkErrorCode::InvalidState,
                    NetworkOperation::Connect,
                    "NetworkRuntime stopped before connect was accepted"));
            }
            mActivityId = activityId.value();
        }
        catch (const std::exception& exception)
        {
            mState.store(TcpClientState::Idle, std::memory_order_release);
            return NetworkResult<void>::failure(makeExceptionError(exception, NetworkOperation::Connect));
        }
        catch (...)
        {
            mState.store(TcpClientState::Idle, std::memory_order_release);
            return NetworkResult<void>::failure(makeUnknownExceptionError(NetworkOperation::Connect));
        }

        NETWORK_TRANSPORT_LOG_INFO("TcpClient", "Connect accepted; generation=" << generation);
        return NetworkResult<void>::success();
    }

    [[nodiscard]] NetworkResult<void> send(ByteBuffer data)
    {
        if (data.empty())
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidArgument,
                NetworkOperation::Write,
                "TCP client cannot send an empty buffer"));
        }

        std::shared_ptr<ByteBuffer> ownedData;
        try
        {
            ownedData = std::make_shared<ByteBuffer>(std::move(data));
        }
        catch (const std::exception& exception)
        {
            return NetworkResult<void>::failure(makeExceptionError(exception, NetworkOperation::Write));
        }

        const std::size_t byteCount = ownedData->size();
        std::scoped_lock lock{mCommandMutex};
        if (!mCore->isRunning())
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Write,
                "NetworkRuntime is not accepting new writes"));
        }
        if (mState.load(std::memory_order_acquire) != TcpClientState::Connected)
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::NotConnected,
                NetworkOperation::Write,
                "TCP client is not connected"));
        }
        if (byteCount > mOptions.maxQueuedWriteBytes - mQueuedWriteBytes)
        {
            NETWORK_TRANSPORT_LOG_DEBUG(
                "TcpClient",
                "Write queue full; requested=" << byteCount << "; queued=" << mQueuedWriteBytes
                    << "; limit=" << mOptions.maxQueuedWriteBytes);
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::QueueFull,
                NetworkOperation::Write,
                "TCP client write queue is full"));
        }

        const std::uint64_t generation = mGeneration.load(std::memory_order_acquire);
        mQueuedWriteBytes += byteCount;
        try
        {
            if (!mCore->postUser(mStrand, [self = shared_from_this(), ownedData = std::move(ownedData), generation]() mutable {
                    self->enqueueWrite(std::move(ownedData), generation);
                }))
            {
                mQueuedWriteBytes -= byteCount;
                return NetworkResult<void>::failure(detail::makeError(
                    NetworkErrorCode::InvalidState,
                    NetworkOperation::Write,
                    "NetworkRuntime stopped before write was scheduled"));
            }
        }
        catch (const std::exception& exception)
        {
            mQueuedWriteBytes -= byteCount;
            return NetworkResult<void>::failure(makeExceptionError(exception, NetworkOperation::Write));
        }
        catch (...)
        {
            mQueuedWriteBytes -= byteCount;
            return NetworkResult<void>::failure(makeUnknownExceptionError(NetworkOperation::Write));
        }
        return NetworkResult<void>::success();
    }

    void close() noexcept
    {
        requestClose(CloseOrigin::Local);
    }

    void dispose() noexcept
    {
        mCallbacksEnabled.store(false, std::memory_order_release);
        requestClose(CloseOrigin::Local);
    }

    [[nodiscard]] TcpClientState state() const noexcept
    {
        return mState.load(std::memory_order_acquire);
    }

    void requestStopFromRuntime() noexcept override
    {
        requestClose(CloseOrigin::RuntimeShutdown);
    }

private:
    [[nodiscard]] bool isCurrent(std::uint64_t generation, TcpClientState expected) const noexcept
    {
        return mGeneration.load(std::memory_order_acquire) == generation
            && mState.load(std::memory_order_acquire) == expected;
    }

    void startConnect(
        Endpoint endpoint,
        ConnectDeadline deadline,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (!isCurrent(generation, TcpClientState::Connecting))
        {
            return;
        }

        try
        {
            // A canceled read handler may outlive its lifecycle. Giving each
            // lifecycle a distinct owning buffer makes immediate reconnect from
            // onClosed safe even before that stale handler is delivered.
            mReadBuffer = std::make_shared<ByteBuffer>(mOptions.readBufferSize);
        }
        catch (const std::exception& exception)
        {
            completeLifecycle(
                generation,
                activityId,
                detail::makeTcpCloseInfo(makeExceptionError(exception, NetworkOperation::Read)));
            return;
        }
        catch (...)
        {
            completeLifecycle(
                generation,
                activityId,
                detail::makeTcpCloseInfo(makeUnknownExceptionError(NetworkOperation::Read)));
            return;
        }

        try
        {
            if (deadline)
            {
                mConnectTimer.expires_at(deadline.value());
                mConnectTimer.async_wait([self = shared_from_this(), generation, activityId](const asio::error_code& error) {
                    if (!error && self->isCurrent(generation, TcpClientState::Connecting))
                    {
                        self->completeLifecycle(
                            generation,
                            activityId,
                            TransportCloseInfo{
                                CloseOrigin::Error,
                                detail::makeError(NetworkErrorCode::TimedOut, NetworkOperation::Connect, "TCP connection timed out"),
                                0});
                    }
                    else if (error != asio::error::operation_aborted
                        && self->isCurrent(generation, TcpClientState::Connecting))
                    {
                        self->completeLifecycle(
                            generation,
                            activityId,
                            detail::makeTcpCloseInfo(makeNetworkError(error, NetworkOperation::Connect)));
                    }
                });
            }

            asio::error_code addressError;
            const asio::ip::address address = asio::ip::make_address(endpoint.host, addressError);
            if (!addressError)
            {
                const tcp::endpoint nativeEndpoint{address, endpoint.port};
                asio::error_code openError;
                mSocket.open(nativeEndpoint.protocol(), openError);
                if (openError)
                {
                    completeLifecycle(
                        generation,
                        activityId,
                        detail::makeTcpCloseInfo(makeNetworkError(openError, NetworkOperation::Connect)));
                    return;
                }
                mSocket.async_connect(
                    nativeEndpoint,
                    [self = shared_from_this(), generation, activityId](const asio::error_code& connectError) {
                        self->handleConnect(connectError, generation, activityId);
                    });
                return;
            }

            mResolver.async_resolve(
                endpoint.host,
                std::to_string(endpoint.port),
                [self = shared_from_this(), generation, activityId](
                    const asio::error_code& error,
                    tcp::resolver::results_type results) mutable {
                    self->handleResolve(error, std::move(results), generation, activityId);
                });
        }
        catch (const std::exception& exception)
        {
            completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(makeExceptionError(exception, NetworkOperation::Resolve)));
        }
        catch (...)
        {
            completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(makeUnknownExceptionError(NetworkOperation::Resolve)));
        }
    }

    void handleResolve(
        const asio::error_code& error,
        tcp::resolver::results_type results,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (!isCurrent(generation, TcpClientState::Connecting))
        {
            return;
        }
        if (error)
        {
            if (error != asio::error::operation_aborted)
            {
                completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(makeNetworkError(error, NetworkOperation::Resolve)));
            }
            return;
        }
        if (results.empty())
        {
            completeLifecycle(
                generation,
                activityId,
                detail::makeTcpCloseInfo(detail::makeError(
                    NetworkErrorCode::NameResolutionFailed,
                    NetworkOperation::Resolve,
                    "TCP endpoint resolved to no addresses")));
            return;
        }

        try
        {
            asio::async_connect(
                mSocket,
                results,
                [self = shared_from_this(), generation, activityId](const asio::error_code& connectError, const tcp::endpoint&) {
                    self->handleConnect(connectError, generation, activityId);
                });
        }
        catch (const std::exception& exception)
        {
            completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(makeExceptionError(exception, NetworkOperation::Connect)));
        }
        catch (...)
        {
            completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(makeUnknownExceptionError(NetworkOperation::Connect)));
        }
    }

    void handleConnect(
        const asio::error_code& error,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (!isCurrent(generation, TcpClientState::Connecting))
        {
            return;
        }
        if (error)
        {
            if (error != asio::error::operation_aborted)
            {
                completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(makeNetworkError(error, NetworkOperation::Connect)));
            }
            return;
        }

        asio::error_code optionError;
        mSocket.set_option(tcp::no_delay{mOptions.noDelay}, optionError);
        if (!optionError)
        {
            mSocket.set_option(asio::socket_base::keep_alive{mOptions.keepAlive}, optionError);
        }
        if (optionError)
        {
            completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(makeNetworkError(optionError, NetworkOperation::Connect)));
            return;
        }

        try
        {
            static_cast<void>(mConnectTimer.cancel());
        }
        catch (...)
        {
        }
        {
            std::scoped_lock lock{mCommandMutex};
            if (!isCurrent(generation, TcpClientState::Connecting))
            {
                return;
            }
            mState.store(TcpClientState::Connected, std::memory_order_release);
        }

        NETWORK_TRANSPORT_LOG_INFO("TcpClient", "Connected; generation=" << generation);
        if (mCallbacksEnabled.load(std::memory_order_acquire)
            && !detail::invokeCallback("TcpClient", "onConnected", mHandlers.onConnected))
        {
            completeLifecycle(
                generation,
                activityId,
                TransportCloseInfo{CloseOrigin::Error, detail::makeCallbackError("onConnected"), 0});
            return;
        }
        if (isCurrent(generation, TcpClientState::Connected))
        {
            startRead(generation, activityId);
        }
    }

    void startRead(std::uint64_t generation, detail::RuntimeActivityId activityId) noexcept
    {
        if (!isCurrent(generation, TcpClientState::Connected))
        {
            return;
        }
        try
        {
            const auto buffer = mReadBuffer;
            if (!buffer)
            {
                completeLifecycle(
                    generation,
                    activityId,
                    TransportCloseInfo{
                        CloseOrigin::Error,
                        detail::makeError(
                            NetworkErrorCode::ResourceExhausted,
                            NetworkOperation::Read,
                            "TCP read buffer is unavailable"),
                        0});
                return;
            }
            mSocket.async_read_some(
                asio::buffer(*buffer),
                [self = shared_from_this(), buffer, generation, activityId](const asio::error_code& error, std::size_t byteCount) {
                    self->handleRead(buffer, error, byteCount, generation, activityId);
                });
        }
        catch (const std::exception& exception)
        {
            completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(makeExceptionError(exception, NetworkOperation::Read)));
        }
        catch (...)
        {
            completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(makeUnknownExceptionError(NetworkOperation::Read)));
        }
    }

    void handleRead(
        const std::shared_ptr<ByteBuffer>& buffer,
        const asio::error_code& error,
        std::size_t byteCount,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (!isCurrent(generation, TcpClientState::Connected))
        {
            return;
        }

        if (byteCount > 0
            && mCallbacksEnabled.load(std::memory_order_acquire)
            && !detail::invokeCallback(
                "TcpClient",
                "onDataReceived",
                mHandlers.onDataReceived,
                ByteView{buffer->data(), byteCount}))
        {
            completeLifecycle(
                generation,
                activityId,
                TransportCloseInfo{CloseOrigin::Error, detail::makeCallbackError("onDataReceived"), 0});
            return;
        }

        if (!isCurrent(generation, TcpClientState::Connected))
        {
            return;
        }
        if (error)
        {
            if (error != asio::error::operation_aborted)
            {
                completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(makeNetworkError(error, NetworkOperation::Read)));
            }
            return;
        }
        startRead(generation, activityId);
    }

    void enqueueWrite(std::shared_ptr<ByteBuffer> data, std::uint64_t generation) noexcept
    {
        if (!isCurrent(generation, TcpClientState::Connected))
        {
            // Admission accounting belongs to the lifecycle that accepted the
            // write. If close() has already claimed that lifecycle, its terminal
            // callback must still report these bytes as discarded. A completed
            // lifecycle has already cleared the counters, and a newer lifecycle
            // has a different generation.
            return;
        }

        const bool writeInProgress = !mWriteQueue.empty();
        try
        {
            mWriteQueue.emplace_back(std::move(data));
        }
        catch (const std::exception& exception)
        {
            completeCurrentWithError(generation, makeExceptionError(exception, NetworkOperation::Write));
            return;
        }
        catch (...)
        {
            completeCurrentWithError(generation, makeUnknownExceptionError(NetworkOperation::Write));
            return;
        }
        if (!writeInProgress)
        {
            startWrite(generation);
        }
    }

    void startWrite(std::uint64_t generation) noexcept
    {
        if (!isCurrent(generation, TcpClientState::Connected) || mWriteQueue.empty())
        {
            return;
        }
        const auto data = mWriteQueue.front();
        try
        {
            asio::async_write(
                mSocket,
                asio::buffer(*data),
                [self = shared_from_this(), data, generation](const asio::error_code& error, std::size_t byteCount) {
                    self->handleWrite(error, byteCount, data->size(), generation);
                });
        }
        catch (const std::exception& exception)
        {
            completeCurrentWithError(generation, makeExceptionError(exception, NetworkOperation::Write));
        }
        catch (...)
        {
            completeCurrentWithError(generation, makeUnknownExceptionError(NetworkOperation::Write));
        }
    }

    void handleWrite(
        const asio::error_code& error,
        std::size_t byteCount,
        std::size_t expectedByteCount,
        std::uint64_t generation) noexcept
    {
        if (mGeneration.load(std::memory_order_acquire) != generation)
        {
            return;
        }
        // async_write may report an error after transferring a prefix. Those
        // bytes are confirmed and must not be counted as terminally discarded,
        // even if close() claimed the lifecycle before this handler ran.
        releaseQueuedBytes(generation, std::min(byteCount, expectedByteCount));
        if (!isCurrent(generation, TcpClientState::Connected))
        {
            // A concurrent close already chose the terminal reason. Its queued
            // actor command will clear the write queue and finish the lifecycle.
            return;
        }
        if (error)
        {
            if (error != asio::error::operation_aborted)
            {
                completeCurrentWithError(generation, makeNetworkError(error, NetworkOperation::Write));
            }
            return;
        }
        if (byteCount != expectedByteCount)
        {
            completeCurrentWithError(
                generation,
                detail::makeError(
                    NetworkErrorCode::SystemError,
                    NetworkOperation::Write,
                    "TCP async_write completed with an unexpected byte count"));
            return;
        }
        mWriteQueue.pop_front();
        startWrite(generation);
    }

    void completeCurrentWithError(std::uint64_t generation, NetworkError error) noexcept
    {
        detail::RuntimeActivityId activityId{0};
        {
            std::scoped_lock lock{mCommandMutex};
            if (mGeneration.load(std::memory_order_acquire) != generation)
            {
                return;
            }
            activityId = mActivityId;
        }
        completeLifecycle(generation, activityId, detail::makeTcpCloseInfo(std::move(error)));
    }

    void requestClose(CloseOrigin origin) noexcept
    {
        std::uint64_t generation{0};
        detail::RuntimeActivityId activityId{0};
        {
            std::scoped_lock lock{mCommandMutex};
            const TcpClientState currentState = mState.load(std::memory_order_acquire);
            if (currentState == TcpClientState::Idle || currentState == TcpClientState::Closing)
            {
                return;
            }
            generation = mGeneration.load(std::memory_order_acquire);
            activityId = mActivityId;
            mState.store(TcpClientState::Closing, std::memory_order_release);
        }

        try
        {
            if (mCore->postInternal(mStrand, [self = shared_from_this(), generation, activityId, origin] {
                    self->completeLifecycle(
                        generation,
                        activityId,
                        TransportCloseInfo{origin, std::nullopt, 0},
                        true);
                }))
            {
                return;
            }
        }
        catch (...)
        {
        }

        // This path is reachable only after a runtime fault, when actor access
        // is no longer possible. Release the lifecycle so shutdown cannot hang.
        abandonLifecycle(generation, activityId);
    }

    void completeLifecycle(
        std::uint64_t generation,
        detail::RuntimeActivityId activityId,
        TransportCloseInfo closeInfo,
        bool closeAlreadyClaimed = false) noexcept
    {
        {
            std::scoped_lock lock{mCommandMutex};
            if (mGeneration.load(std::memory_order_acquire) != generation)
            {
                return;
            }
            const TcpClientState currentState = mState.load(std::memory_order_acquire);
            if (currentState == TcpClientState::Idle
                || (closeAlreadyClaimed && currentState != TcpClientState::Closing)
                || (!closeAlreadyClaimed && currentState == TcpClientState::Closing))
            {
                return;
            }
            // Claim the terminal transition before touching the socket. This
            // closes send()/connect() admission and makes the first close reason
            // win against concurrent user or runtime shutdown requests.
            mState.store(TcpClientState::Closing, std::memory_order_release);
        }

        try
        {
            static_cast<void>(mConnectTimer.cancel());
        }
        catch (...)
        {
        }
        asio::error_code ignoredError;
        try
        {
            mResolver.cancel();
        }
        catch (...)
        {
        }
        mSocket.cancel(ignoredError);
        mSocket.shutdown(tcp::socket::shutdown_both, ignoredError);
        mSocket.close(ignoredError);
        mReadBuffer.reset();
        mWriteQueue.clear();

        {
            std::scoped_lock lock{mCommandMutex};
            if (mGeneration.load(std::memory_order_acquire) != generation
                || mState.load(std::memory_order_acquire) != TcpClientState::Closing)
            {
                return;
            }
            closeInfo.discardedWriteBytes = mQueuedWriteBytes;
            mQueuedWriteBytes = 0;
            if (mActivityId == activityId)
            {
                mActivityId = 0;
            }
            mState.store(TcpClientState::Idle, std::memory_order_release);
        }

        if (closeInfo.origin == CloseOrigin::Error)
        {
            NETWORK_TRANSPORT_LOG_WARN(
                "TcpClient",
                "Lifecycle failed; generation=" << generation
                    << "; code=" << (closeInfo.cause ? detail::toString(closeInfo.cause->code) : "Unknown")
                    << "; operation=" << (closeInfo.cause ? detail::toString(closeInfo.cause->operation) : "None")
                    << "; cause=" << (closeInfo.cause ? closeInfo.cause->diagnostic : std::string{"unavailable"})
                    << "; discardedWriteBytes=" << closeInfo.discardedWriteBytes);
        }
        else
        {
            NETWORK_TRANSPORT_LOG_INFO(
                "TcpClient",
                "Lifecycle closed; generation=" << generation
                    << "; origin=" << detail::toString(closeInfo.origin)
                    << "; discardedWriteBytes=" << closeInfo.discardedWriteBytes);
        }
        if (mCallbacksEnabled.load(std::memory_order_acquire))
        {
            static_cast<void>(detail::invokeCallback("TcpClient", "onClosed", mHandlers.onClosed, closeInfo));
        }
        mCore->finishActivity(activityId);
    }

    void abandonLifecycle(std::uint64_t generation, detail::RuntimeActivityId activityId) noexcept
    {
        {
            std::scoped_lock lock{mCommandMutex};
            if (mGeneration.load(std::memory_order_acquire) == generation)
            {
                mQueuedWriteBytes = 0;
                if (mActivityId == activityId)
                {
                    mActivityId = 0;
                }
                mState.store(TcpClientState::Idle, std::memory_order_release);
            }
        }
        mCore->finishActivity(activityId);
    }

    void releaseQueuedBytes(std::uint64_t generation, std::size_t byteCount) noexcept
    {
        std::scoped_lock lock{mCommandMutex};
        if (mGeneration.load(std::memory_order_acquire) == generation)
        {
            mQueuedWriteBytes = byteCount <= mQueuedWriteBytes ? mQueuedWriteBytes - byteCount : 0;
        }
    }

private:
    std::shared_ptr<detail::RuntimeCore> mCore;
    asio::strand<asio::any_io_executor> mStrand;
    tcp::resolver mResolver;
    tcp::socket mSocket;
    asio::steady_timer mConnectTimer;
    TcpClientOptions mOptions;
    TcpClientHandlers mHandlers;
    // Actor-owned pointer for the current lifecycle. Each read handler captures
    // its own copy, so a stale canceled handler never aliases a reconnect buffer.
    std::shared_ptr<ByteBuffer> mReadBuffer;
    std::deque<std::shared_ptr<ByteBuffer>> mWriteQueue;

    std::atomic<TcpClientState> mState{TcpClientState::Idle};
    std::atomic<std::uint64_t> mGeneration{0};
    std::atomic<bool> mCallbacksEnabled{true};
    mutable std::mutex mCommandMutex;
    detail::RuntimeActivityId mActivityId{0};
    std::size_t mQueuedWriteBytes{0};
};

TcpClient::TcpClient(NetworkRuntime& runtime, TcpClientHandlers handlers)
    : TcpClient{runtime, TcpClientOptions{}, std::move(handlers)}
{
}

TcpClient::TcpClient(NetworkRuntime& runtime, TcpClientOptions options, TcpClientHandlers handlers)
    : mImpl{std::make_shared<Impl>(runtime, options, std::move(handlers))}
{
}

TcpClient::~TcpClient()
{
    mImpl->dispose();
}

NetworkResult<void> TcpClient::connect(Endpoint endpoint)
{
    return mImpl->connect(std::move(endpoint));
}

NetworkResult<void> TcpClient::send(ByteBuffer data)
{
    return mImpl->send(std::move(data));
}

void TcpClient::close() noexcept
{
    mImpl->close();
}

TcpClientState TcpClient::state() const noexcept
{
    return mImpl->state();
}

} // namespace ucf::utilities::network
