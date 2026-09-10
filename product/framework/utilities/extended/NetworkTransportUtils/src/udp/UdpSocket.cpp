#include <ucf/utilities/NetworkTransportUtils/udp/UdpSocket.h>

#include "NetworkTransportLogger.h"
#include "core/AsioErrorMapper.h"
#include "core/CallbackUtils.h"
#include "core/EndpointUtils.h"
#include "core/ExceptionErrorMapper.h"
#include "core/NetworkEnumStrings.h"
#include "core/NetworkErrorUtils.h"
#include "core/SteadyClockUtils.h"
#include "runtime/NetworkRuntimeAccess.h"
#include "runtime/RuntimeCore.h"

#include <asio/buffer.hpp>
#include <asio/error.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/udp.hpp>
#include <asio/strand.hpp>

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
#include <unordered_map>
#include <utility>

namespace ucf::utilities::network {

namespace {

using asio::ip::udp;

[[nodiscard]] bool isRetryableReceiveError(const asio::error_code& error) noexcept
{
    return error == asio::error::try_again
        || error == asio::error::would_block
        || error == asio::error::interrupted
        || error == asio::error::no_buffer_space;
}

[[nodiscard]] UdpSocketOptions validateOptions(UdpSocketOptions options)
{
    if (options.maxDatagramSize == 0
        || options.maxQueuedSendBytes == 0
        || options.maxQueuedDatagrams == 0)
    {
        throw std::invalid_argument{"UdpSocket queue and datagram limits must be greater than zero"};
    }
    if (options.maxDatagramSize > 65'507)
    {
        throw std::invalid_argument{"UdpSocket maxDatagramSize exceeds the portable UDP payload limit"};
    }
    if (options.dnsCacheTtl < std::chrono::milliseconds::zero())
    {
        throw std::invalid_argument{"UdpSocket dnsCacheTtl must not be negative"};
    }
    return options;
}

} // namespace

class UdpSocket::Impl final
    : public detail::RuntimeParticipant
    , public std::enable_shared_from_this<UdpSocket::Impl>
{
private:
    struct ReceiveContext final
    {
        explicit ReceiveContext(std::size_t bufferSize)
            : buffer(bufferSize)
        {
        }

        ByteBuffer buffer;
        udp::endpoint sender;
    };

    struct PendingDatagram final
    {
        Endpoint destination;
        ByteBuffer data;
        udp::endpoint nativeDestination;
    };

    struct CachedEndpoint final
    {
        udp::endpoint endpoint;
        std::chrono::steady_clock::time_point expiresAt;
    };

public:
    Impl(NetworkRuntime& runtime, UdpSocketOptions options, UdpSocketHandlers handlers)
        : mCore{detail::NetworkRuntimeAccess::core(runtime)}
        , mStrand{asio::make_strand(mCore->executor())}
        , mResolver{mStrand}
        , mSocket{mStrand}
        , mOptions{validateOptions(options)}
        , mHandlers{std::move(handlers)}
    {
    }

    [[nodiscard]] NetworkResult<void> bind(Endpoint endpoint)
    {
        std::scoped_lock lock{mCommandMutex};
        if (!mCore->isRunning())
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Bind,
                "NetworkRuntime is not accepting new work"));
        }
        if (mState.load(std::memory_order_acquire) != UdpSocketState::Idle)
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Bind,
                "UDP socket already has an active lifecycle"));
        }

        const std::uint64_t generation = mGeneration.fetch_add(1, std::memory_order_acq_rel) + 1;
        mState.store(UdpSocketState::Binding, std::memory_order_release);
        try
        {
            const auto activityId = mCore->startActivity(
                shared_from_this(),
                mStrand,
                [self = shared_from_this(), endpoint = std::move(endpoint), generation](detail::RuntimeActivityId id) mutable {
                    self->startBind(std::move(endpoint), generation, id);
                });
            if (!activityId)
            {
                mState.store(UdpSocketState::Idle, std::memory_order_release);
                return NetworkResult<void>::failure(detail::makeError(
                    NetworkErrorCode::InvalidState,
                    NetworkOperation::Bind,
                    "NetworkRuntime stopped before UDP bind was accepted"));
            }
            mActivityId = activityId.value();
        }
        catch (const std::exception& exception)
        {
            mState.store(UdpSocketState::Idle, std::memory_order_release);
            return NetworkResult<void>::failure(makeExceptionError(exception, NetworkOperation::Bind));
        }
        catch (...)
        {
            mState.store(UdpSocketState::Idle, std::memory_order_release);
            return NetworkResult<void>::failure(makeUnknownExceptionError(NetworkOperation::Bind));
        }

        NETWORK_TRANSPORT_LOG_INFO("UdpSocket", "Bind accepted; generation=" << generation);
        return NetworkResult<void>::success();
    }

    [[nodiscard]] NetworkResult<void> sendTo(Endpoint destination, ByteBuffer data)
    {
        if (destination.host.empty() || destination.port == 0)
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidArgument,
                NetworkOperation::Send,
                "UDP destination host and port must be specified"));
        }
        if (data.size() > mOptions.maxDatagramSize)
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::MessageTooLarge,
                NetworkOperation::Send,
                "UDP datagram exceeds maxDatagramSize"));
        }

        std::shared_ptr<PendingDatagram> datagram;
        try
        {
            datagram = std::make_shared<PendingDatagram>(PendingDatagram{
                std::move(destination),
                std::move(data),
                {}});
        }
        catch (const std::exception& exception)
        {
            return NetworkResult<void>::failure(makeExceptionError(exception, NetworkOperation::Send));
        }

        const std::size_t byteCount = datagram->data.size();
        std::scoped_lock lock{mCommandMutex};
        if (!mCore->isRunning())
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Send,
                "NetworkRuntime is not accepting new sends"));
        }
        if (mState.load(std::memory_order_acquire) != UdpSocketState::Bound)
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Send,
                "UDP socket is not bound"));
        }
        if (mQueuedDatagramCount >= mOptions.maxQueuedDatagrams
            || byteCount > mOptions.maxQueuedSendBytes - mQueuedSendBytes)
        {
            NETWORK_TRANSPORT_LOG_DEBUG(
                "UdpSocket",
                "Send queue full; requestedBytes=" << byteCount
                    << "; queuedBytes=" << mQueuedSendBytes
                    << "; queuedDatagrams=" << mQueuedDatagramCount);
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::QueueFull,
                NetworkOperation::Send,
                "UDP send queue is full"));
        }

        const std::uint64_t generation = mGeneration.load(std::memory_order_acquire);
        mQueuedSendBytes += byteCount;
        ++mQueuedDatagramCount;
        try
        {
            if (!mCore->postUser(mStrand, [self = shared_from_this(), datagram = std::move(datagram), generation]() mutable {
                    self->enqueueSend(std::move(datagram), generation);
                }))
            {
                mQueuedSendBytes -= byteCount;
                --mQueuedDatagramCount;
                return NetworkResult<void>::failure(detail::makeError(
                    NetworkErrorCode::InvalidState,
                    NetworkOperation::Send,
                    "NetworkRuntime stopped before UDP send was scheduled"));
            }
        }
        catch (const std::exception& exception)
        {
            mQueuedSendBytes -= byteCount;
            --mQueuedDatagramCount;
            return NetworkResult<void>::failure(makeExceptionError(exception, NetworkOperation::Send));
        }
        catch (...)
        {
            mQueuedSendBytes -= byteCount;
            --mQueuedDatagramCount;
            return NetworkResult<void>::failure(makeUnknownExceptionError(NetworkOperation::Send));
        }
        return NetworkResult<void>::success();
    }

    void close() noexcept
    {
        requestClose(TransportCloseInfo{CloseOrigin::Local, std::nullopt, 0});
    }

    void dispose() noexcept
    {
        mCallbacksEnabled.store(false, std::memory_order_release);
        close();
    }

    [[nodiscard]] UdpSocketState state() const noexcept
    {
        return mState.load(std::memory_order_acquire);
    }

    void requestStopFromRuntime() noexcept override
    {
        requestClose(TransportCloseInfo{CloseOrigin::RuntimeShutdown, std::nullopt, 0});
    }

private:
    [[nodiscard]] bool isState(std::uint64_t generation, UdpSocketState state) const noexcept
    {
        return mGeneration.load(std::memory_order_acquire) == generation
            && mState.load(std::memory_order_acquire) == state;
    }

    void startBind(
        Endpoint endpoint,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (!isState(generation, UdpSocketState::Binding))
        {
            return;
        }
        if (endpoint.host.empty())
        {
            openAndBind(udp::endpoint{udp::v4(), endpoint.port}, generation, activityId);
            return;
        }

        asio::error_code addressError;
        const asio::ip::address address = asio::ip::make_address(endpoint.host, addressError);
        if (!addressError)
        {
            openAndBind(udp::endpoint{address, endpoint.port}, generation, activityId);
            return;
        }

        try
        {
            mResolver.async_resolve(
                endpoint.host,
                std::to_string(endpoint.port),
                udp::resolver::passive,
                [self = shared_from_this(), generation, activityId](
                    const asio::error_code& error,
                    udp::resolver::results_type results) mutable {
                    self->handleBindResolve(error, std::move(results), generation, activityId);
                });
        }
        catch (const std::exception& exception)
        {
            failBind(generation, activityId, makeExceptionError(exception, NetworkOperation::Resolve));
        }
        catch (...)
        {
            failBind(generation, activityId, makeUnknownExceptionError(NetworkOperation::Resolve));
        }
    }

    void handleBindResolve(
        const asio::error_code& error,
        udp::resolver::results_type results,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (!isState(generation, UdpSocketState::Binding))
        {
            return;
        }
        if (error)
        {
            if (error != asio::error::operation_aborted)
            {
                failBind(generation, activityId, makeNetworkError(error, NetworkOperation::Resolve));
            }
            return;
        }
        if (results.empty())
        {
            failBind(
                generation,
                activityId,
                detail::makeError(
                    NetworkErrorCode::NameResolutionFailed,
                    NetworkOperation::Resolve,
                    "UDP bind endpoint resolved to no addresses"));
            return;
        }
        openAndBind(results.begin()->endpoint(), generation, activityId);
    }

    void openAndBind(
        const udp::endpoint& endpoint,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        asio::error_code error;
        mSocket.open(endpoint.protocol(), error);
        if (!error)
        {
            mSocket.set_option(udp::socket::reuse_address{mOptions.reuseAddress}, error);
        }
        if (!error)
        {
            mSocket.bind(endpoint, error);
        }
        if (error)
        {
            failBind(generation, activityId, makeNetworkError(error, NetworkOperation::Bind));
            return;
        }

        const udp::endpoint localEndpoint = mSocket.local_endpoint(error);
        if (error)
        {
            failBind(generation, activityId, makeNetworkError(error, NetworkOperation::Bind));
            return;
        }
        auto publicEndpoint = detail::toPublicEndpoint(localEndpoint, NetworkOperation::Bind);
        if (!publicEndpoint)
        {
            failBind(generation, activityId, publicEndpoint.error());
            return;
        }

        std::shared_ptr<ReceiveContext> receiveContext;
        try
        {
            // One extra byte detects platforms that silently truncate oversized
            // datagrams instead of reporting asio::error::message_size. A fresh
            // context per bind also isolates stale canceled receive handlers.
            receiveContext = std::make_shared<ReceiveContext>(mOptions.maxDatagramSize + 1);
        }
        catch (const std::exception& exception)
        {
            failBind(generation, activityId, makeExceptionError(exception, NetworkOperation::Receive));
            return;
        }
        catch (...)
        {
            failBind(generation, activityId, makeUnknownExceptionError(NetworkOperation::Receive));
            return;
        }

        {
            std::scoped_lock lock{mCommandMutex};
            if (!isState(generation, UdpSocketState::Binding))
            {
                return;
            }
            mReceiveContext = std::move(receiveContext);
            mUsesIpv6 = endpoint.protocol() == udp::v6();
            mState.store(UdpSocketState::Bound, std::memory_order_release);
        }

        const Endpoint boundEndpoint = std::move(publicEndpoint).value();
        NETWORK_TRANSPORT_LOG_INFO(
            "UdpSocket",
            "Bound to " << boundEndpoint.host << ":" << boundEndpoint.port
                << "; generation=" << generation);
        if (mCallbacksEnabled.load(std::memory_order_acquire)
            && !detail::invokeCallback("UdpSocket", "onBound", mHandlers.onBound, boundEndpoint))
        {
            requestClose(TransportCloseInfo{
                CloseOrigin::Error,
                detail::makeCallbackError("onBound"),
                0});
            return;
        }
        if (isState(generation, UdpSocketState::Bound))
        {
            startReceive(generation, activityId);
        }
    }

    void startReceive(std::uint64_t generation, detail::RuntimeActivityId activityId) noexcept
    {
        if (!isState(generation, UdpSocketState::Bound))
        {
            return;
        }
        const auto context = mReceiveContext;
        if (!context)
        {
            terminateReceive(
                generation,
                activityId,
                detail::makeError(
                    NetworkErrorCode::ResourceExhausted,
                    NetworkOperation::Receive,
                    "UDP receive context is unavailable"));
            return;
        }
        try
        {
            mSocket.async_receive_from(
                asio::buffer(context->buffer),
                context->sender,
                [self = shared_from_this(), context, generation, activityId](const asio::error_code& error, std::size_t byteCount) {
                    self->handleReceive(context, error, byteCount, generation, activityId);
                });
        }
        catch (const std::exception& exception)
        {
            terminateReceive(generation, activityId, makeExceptionError(exception, NetworkOperation::Receive));
        }
        catch (...)
        {
            terminateReceive(generation, activityId, makeUnknownExceptionError(NetworkOperation::Receive));
        }
    }

    void handleReceive(
        const std::shared_ptr<ReceiveContext>& context,
        const asio::error_code& error,
        std::size_t byteCount,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (!isState(generation, UdpSocketState::Bound))
        {
            return;
        }
        if (error)
        {
            if (error == asio::error::operation_aborted)
            {
                return;
            }
            if (error == asio::error::message_size)
            {
                if (!notifyReceiveError(detail::makeError(
                        NetworkErrorCode::MessageTooLarge,
                        NetworkOperation::Receive,
                        "Received UDP datagram exceeds maxDatagramSize")))
                {
                    return;
                }
                startReceive(generation, activityId);
                return;
            }
            if (isRetryableReceiveError(error))
            {
                if (!notifyReceiveError(makeNetworkError(error, NetworkOperation::Receive)))
                {
                    return;
                }
                startReceive(generation, activityId);
                return;
            }
            terminateReceive(generation, activityId, makeNetworkError(error, NetworkOperation::Receive));
            return;
        }

        if (byteCount > mOptions.maxDatagramSize)
        {
            if (notifyReceiveError(detail::makeError(
                    NetworkErrorCode::MessageTooLarge,
                    NetworkOperation::Receive,
                    "Received UDP datagram exceeds maxDatagramSize")))
            {
                startReceive(generation, activityId);
            }
            return;
        }

        auto sender = detail::toPublicEndpoint(context->sender, NetworkOperation::Receive);
        if (!sender)
        {
            if (notifyReceiveError(sender.error()))
            {
                startReceive(generation, activityId);
            }
            return;
        }

        if (mCallbacksEnabled.load(std::memory_order_acquire)
            && !detail::invokeCallback(
                "UdpSocket",
                "onDatagramReceived",
                mHandlers.onDatagramReceived,
                sender.value(),
                ByteView{context->buffer.data(), byteCount}))
        {
            requestClose(TransportCloseInfo{
                CloseOrigin::Error,
                detail::makeCallbackError("onDatagramReceived"),
                0});
            return;
        }
        if (isState(generation, UdpSocketState::Bound))
        {
            startReceive(generation, activityId);
        }
    }

    [[nodiscard]] bool notifyReceiveError(const NetworkError& error) noexcept
    {
        NETWORK_TRANSPORT_LOG_WARN(
            "UdpSocket",
            "Recoverable receive error; code=" << detail::toString(error.code)
                << "; cause=" << error.diagnostic);
        if (!mCallbacksEnabled.load(std::memory_order_acquire))
        {
            return true;
        }
        if (detail::invokeCallback("UdpSocket", "onReceiveError", mHandlers.onReceiveError, error))
        {
            return true;
        }
        requestClose(TransportCloseInfo{
            CloseOrigin::Error,
            detail::makeCallbackError("onReceiveError"),
            0});
        return false;
    }

    void terminateReceive(
        std::uint64_t generation,
        detail::RuntimeActivityId activityId,
        NetworkError error) noexcept
    {
        if (!isState(generation, UdpSocketState::Bound))
        {
            return;
        }
        static_cast<void>(activityId);
        NETWORK_TRANSPORT_LOG_ERROR("UdpSocket", "Receive loop terminated: " << error.diagnostic);
        requestClose(TransportCloseInfo{CloseOrigin::Error, std::move(error), 0});
    }

    void enqueueSend(std::shared_ptr<PendingDatagram> datagram, std::uint64_t generation) noexcept
    {
        if (!isState(generation, UdpSocketState::Bound))
        {
            // If close() already claimed this generation, retain the admission
            // counters so onClosed includes this accepted datagram in
            // discardedWriteBytes. A completed or newer lifecycle cannot reuse
            // these counters because finishClose clears them and generation
            // changes on bind().
            return;
        }
        const bool sendInProgress = !mSendQueue.empty();
        const std::size_t byteCount = datagram->data.size();
        try
        {
            mSendQueue.emplace_back(datagram);
        }
        catch (const std::exception& exception)
        {
            releaseQueuedCapacity(generation, byteCount);
            static_cast<void>(notifySendError(datagram->destination, makeExceptionError(exception, NetworkOperation::Send)));
            return;
        }
        catch (...)
        {
            releaseQueuedCapacity(generation, byteCount);
            static_cast<void>(notifySendError(datagram->destination, makeUnknownExceptionError(NetworkOperation::Send)));
            return;
        }
        if (!sendInProgress)
        {
            startNextSend(generation);
        }
    }

    void startNextSend(std::uint64_t generation) noexcept
    {
        if (!isState(generation, UdpSocketState::Bound) || mSendQueue.empty())
        {
            return;
        }
        const auto& datagram = mSendQueue.front();

        asio::error_code addressError;
        const asio::ip::address address = asio::ip::make_address(datagram->destination.host, addressError);
        if (!addressError)
        {
            if (address.is_v6() != mUsesIpv6)
            {
                completeCurrentSend(
                    generation,
                    detail::makeError(
                        NetworkErrorCode::AddressNotAvailable,
                        NetworkOperation::Resolve,
                        "UDP destination address family does not match the bound socket"));
                return;
            }
            datagram->nativeDestination = udp::endpoint{address, datagram->destination.port};
            beginNativeSend(datagram, generation);
            return;
        }

        std::string cacheKey;
        try
        {
            const bool cacheEnabled = mOptions.dnsCacheTtl > std::chrono::milliseconds::zero()
                && mOptions.maxDnsCacheEntries > 0;
            if (cacheEnabled)
            {
                cacheKey = datagram->destination.host + ":" + std::to_string(datagram->destination.port);
                const auto iterator = mDnsCache.find(cacheKey);
                if (iterator != mDnsCache.end())
                {
                    if (iterator->second.expiresAt > std::chrono::steady_clock::now())
                    {
                        datagram->nativeDestination = iterator->second.endpoint;
                        beginNativeSend(datagram, generation);
                        return;
                    }
                    mDnsCache.erase(iterator);
                }
            }

            const udp protocol = mUsesIpv6 ? udp::v6() : udp::v4();
            mResolver.async_resolve(
                protocol,
                datagram->destination.host,
                std::to_string(datagram->destination.port),
                [self = shared_from_this(), cacheKey = std::move(cacheKey), generation](
                    const asio::error_code& error,
                    udp::resolver::results_type results) mutable {
                    self->handleSendResolve(error, std::move(results), std::move(cacheKey), generation);
                });
        }
        catch (const std::exception& exception)
        {
            completeCurrentSend(generation, makeExceptionError(exception, NetworkOperation::Resolve));
        }
        catch (...)
        {
            completeCurrentSend(generation, makeUnknownExceptionError(NetworkOperation::Resolve));
        }
    }

    void handleSendResolve(
        const asio::error_code& error,
        udp::resolver::results_type results,
        std::string cacheKey,
        std::uint64_t generation) noexcept
    {
        if (!isState(generation, UdpSocketState::Bound) || mSendQueue.empty())
        {
            return;
        }
        if (error)
        {
            if (error != asio::error::operation_aborted)
            {
                completeCurrentSend(generation, makeNetworkError(error, NetworkOperation::Resolve));
            }
            return;
        }
        if (results.empty())
        {
            completeCurrentSend(
                generation,
                detail::makeError(
                    NetworkErrorCode::NameResolutionFailed,
                    NetworkOperation::Resolve,
                    "UDP destination resolved to no addresses"));
            return;
        }

        const auto datagram = mSendQueue.front();
        datagram->nativeDestination = results.begin()->endpoint();
        if (mOptions.dnsCacheTtl > std::chrono::milliseconds::zero()
            && mOptions.maxDnsCacheEntries > 0)
        {
            try
            {
                const auto now = std::chrono::steady_clock::now();
                for (auto iterator = mDnsCache.begin(); iterator != mDnsCache.end();)
                {
                    if (iterator->second.expiresAt <= now)
                    {
                        iterator = mDnsCache.erase(iterator);
                    }
                    else
                    {
                        ++iterator;
                    }
                }
                if (mDnsCache.size() >= mOptions.maxDnsCacheEntries)
                {
                    mDnsCache.erase(mDnsCache.begin());
                }
                mDnsCache.insert_or_assign(
                    std::move(cacheKey),
                    CachedEndpoint{
                        datagram->nativeDestination,
                        detail::steadyDeadlineAfter(mOptions.dnsCacheTtl)});
            }
            catch (...)
            {
                // Caching is an optimization; delivery must not fail because
                // the cache could not allocate.
            }
        }
        beginNativeSend(datagram, generation);
    }

    void beginNativeSend(
        const std::shared_ptr<PendingDatagram>& datagram,
        std::uint64_t generation) noexcept
    {
        try
        {
            mSocket.async_send_to(
                asio::buffer(datagram->data),
                datagram->nativeDestination,
                [self = shared_from_this(), datagram, generation](const asio::error_code& error, std::size_t byteCount) {
                    self->handleSend(error, byteCount, datagram->data.size(), generation);
                });
        }
        catch (const std::exception& exception)
        {
            completeCurrentSend(generation, makeExceptionError(exception, NetworkOperation::Send));
        }
        catch (...)
        {
            completeCurrentSend(generation, makeUnknownExceptionError(NetworkOperation::Send));
        }
    }

    void handleSend(
        const asio::error_code& error,
        std::size_t byteCount,
        std::size_t expectedByteCount,
        std::uint64_t generation) noexcept
    {
        if (mGeneration.load(std::memory_order_acquire) != generation)
        {
            return;
        }
        if (mState.load(std::memory_order_acquire) != UdpSocketState::Bound)
        {
            // Completion can race with a synchronously claimed close. Preserve
            // first-terminal-wins while excluding any confirmed bytes from the
            // close callback's discarded-byte total.
            if (byteCount > 0)
            {
                releaseQueuedCapacity(generation, std::min(byteCount, expectedByteCount));
            }
            return;
        }
        if (error)
        {
            if (error != asio::error::operation_aborted)
            {
                completeCurrentSend(generation, makeNetworkError(error, NetworkOperation::Send));
            }
            return;
        }
        if (byteCount != expectedByteCount)
        {
            completeCurrentSend(
                generation,
                detail::makeError(NetworkErrorCode::SystemError, NetworkOperation::Send, "UDP socket reported a partial datagram send"));
            return;
        }
        completeCurrentSend(generation, std::nullopt);
    }

    void completeCurrentSend(
        std::uint64_t generation,
        std::optional<NetworkError> error) noexcept
    {
        if (!isState(generation, UdpSocketState::Bound) || mSendQueue.empty())
        {
            return;
        }
        const auto datagram = mSendQueue.front();
        mSendQueue.pop_front();
        releaseQueuedCapacity(generation, datagram->data.size());
        if (error && !notifySendError(datagram->destination, error.value()))
        {
            return;
        }
        startNextSend(generation);
    }

    [[nodiscard]] bool notifySendError(
        const Endpoint& destination,
        const NetworkError& error) noexcept
    {
        NETWORK_TRANSPORT_LOG_WARN(
            "UdpSocket",
            "Send failed to " << destination.host << ":" << destination.port << ": " << error.diagnostic);
        if (!mCallbacksEnabled.load(std::memory_order_acquire))
        {
            return true;
        }
        if (detail::invokeCallback("UdpSocket", "onSendError", mHandlers.onSendError, destination, error))
        {
            return true;
        }
        requestClose(TransportCloseInfo{
            CloseOrigin::Error,
            detail::makeCallbackError("onSendError"),
            0});
        return false;
    }

    void failBind(
        std::uint64_t generation,
        detail::RuntimeActivityId activityId,
        NetworkError error) noexcept
    {
        if (!isState(generation, UdpSocketState::Binding))
        {
            return;
        }
        {
            std::scoped_lock lock{mCommandMutex};
            if (!isState(generation, UdpSocketState::Binding))
            {
                return;
            }
            mState.store(UdpSocketState::Closing, std::memory_order_release);
        }
        finishClose(
            generation,
            activityId,
            TransportCloseInfo{CloseOrigin::Error, std::move(error), 0});
    }

    void requestClose(TransportCloseInfo closeInfo) noexcept
    {
        std::uint64_t generation{0};
        detail::RuntimeActivityId activityId{0};
        {
            std::scoped_lock lock{mCommandMutex};
            const UdpSocketState currentState = mState.load(std::memory_order_acquire);
            if (currentState == UdpSocketState::Idle || currentState == UdpSocketState::Closing)
            {
                return;
            }
            generation = mGeneration.load(std::memory_order_acquire);
            activityId = mActivityId;
            mState.store(UdpSocketState::Closing, std::memory_order_release);
        }

        try
        {
            if (mCore->postInternal(
                    mStrand,
                    [self = shared_from_this(), generation, activityId, closeInfo = std::move(closeInfo)]() mutable {
                        self->finishClose(generation, activityId, std::move(closeInfo));
                    }))
            {
                return;
            }
        }
        catch (...)
        {
        }
        abandonLifecycle(generation, activityId);
    }

    void finishClose(
        std::uint64_t generation,
        detail::RuntimeActivityId activityId,
        TransportCloseInfo closeInfo) noexcept
    {
        if (mGeneration.load(std::memory_order_acquire) != generation
            || mState.load(std::memory_order_acquire) == UdpSocketState::Idle)
        {
            return;
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
        mSocket.close(ignoredError);
        mReceiveContext.reset();
        mSendQueue.clear();
        mDnsCache.clear();

        {
            std::scoped_lock lock{mCommandMutex};
            if (mGeneration.load(std::memory_order_acquire) != generation
                || mState.load(std::memory_order_acquire) == UdpSocketState::Idle)
            {
                return;
            }
            closeInfo.discardedWriteBytes = mQueuedSendBytes;
            mQueuedSendBytes = 0;
            mQueuedDatagramCount = 0;
            if (mActivityId == activityId)
            {
                mActivityId = 0;
            }
            mState.store(UdpSocketState::Idle, std::memory_order_release);
        }

        if (closeInfo.origin == CloseOrigin::Error)
        {
            NETWORK_TRANSPORT_LOG_WARN(
                "UdpSocket",
                "Lifecycle failed; generation=" << generation
                    << "; code=" << (closeInfo.cause ? detail::toString(closeInfo.cause->code) : "Unknown")
                    << "; operation=" << (closeInfo.cause ? detail::toString(closeInfo.cause->operation) : "None")
                    << "; cause=" << (closeInfo.cause ? closeInfo.cause->diagnostic : std::string{"unavailable"})
                    << "; discardedSendBytes=" << closeInfo.discardedWriteBytes);
        }
        else
        {
            NETWORK_TRANSPORT_LOG_INFO(
                "UdpSocket",
                "Closed; generation=" << generation
                    << "; origin=" << detail::toString(closeInfo.origin)
                    << "; discardedSendBytes=" << closeInfo.discardedWriteBytes);
        }
        if (mCallbacksEnabled.load(std::memory_order_acquire))
        {
            static_cast<void>(detail::invokeCallback("UdpSocket", "onClosed", mHandlers.onClosed, closeInfo));
        }
        mCore->finishActivity(activityId);
    }

    void abandonLifecycle(
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        {
            std::scoped_lock lock{mCommandMutex};
            if (mGeneration.load(std::memory_order_acquire) == generation)
            {
                mQueuedSendBytes = 0;
                mQueuedDatagramCount = 0;
                if (mActivityId == activityId)
                {
                    mActivityId = 0;
                }
                mState.store(UdpSocketState::Idle, std::memory_order_release);
            }
        }
        mCore->finishActivity(activityId);
    }

    void releaseQueuedCapacity(std::uint64_t generation, std::size_t byteCount) noexcept
    {
        std::scoped_lock lock{mCommandMutex};
        if (mGeneration.load(std::memory_order_acquire) != generation)
        {
            return;
        }
        mQueuedSendBytes = byteCount <= mQueuedSendBytes ? mQueuedSendBytes - byteCount : 0;
        if (mQueuedDatagramCount > 0)
        {
            --mQueuedDatagramCount;
        }
    }

private:
    std::shared_ptr<detail::RuntimeCore> mCore;
    asio::strand<asio::any_io_executor> mStrand;
    udp::resolver mResolver;
    udp::socket mSocket;
    UdpSocketOptions mOptions;
    UdpSocketHandlers mHandlers;
    // One context per bind lifecycle; outstanding canceled handlers retain the
    // old context while a rebind can immediately install a new one.
    std::shared_ptr<ReceiveContext> mReceiveContext;
    std::deque<std::shared_ptr<PendingDatagram>> mSendQueue;
    std::unordered_map<std::string, CachedEndpoint> mDnsCache;

    std::atomic<UdpSocketState> mState{UdpSocketState::Idle};
    std::atomic<std::uint64_t> mGeneration{0};
    std::atomic<bool> mCallbacksEnabled{true};
    mutable std::mutex mCommandMutex;
    detail::RuntimeActivityId mActivityId{0};
    std::size_t mQueuedSendBytes{0};
    std::size_t mQueuedDatagramCount{0};
    bool mUsesIpv6{false};
};

UdpSocket::UdpSocket(NetworkRuntime& runtime, UdpSocketHandlers handlers)
    : UdpSocket{runtime, UdpSocketOptions{}, std::move(handlers)}
{
}

UdpSocket::UdpSocket(NetworkRuntime& runtime, UdpSocketOptions options, UdpSocketHandlers handlers)
    : mImpl{std::make_shared<Impl>(runtime, options, std::move(handlers))}
{
}

UdpSocket::~UdpSocket()
{
    mImpl->dispose();
}

NetworkResult<void> UdpSocket::bind(Endpoint localEndpoint)
{
    return mImpl->bind(std::move(localEndpoint));
}

NetworkResult<void> UdpSocket::sendTo(Endpoint destination, ByteBuffer data)
{
    return mImpl->sendTo(std::move(destination), std::move(data));
}

void UdpSocket::close() noexcept
{
    mImpl->close();
}

UdpSocketState UdpSocket::state() const noexcept
{
    return mImpl->state();
}

} // namespace ucf::utilities::network
