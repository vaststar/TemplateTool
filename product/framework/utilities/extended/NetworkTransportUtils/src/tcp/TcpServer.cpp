#include <ucf/utilities/NetworkTransportUtils/tcp/TcpServer.h>

#include "NetworkTransportLogger.h"
#include "core/AsioErrorMapper.h"
#include "core/CallbackUtils.h"
#include "core/EndpointUtils.h"
#include "core/ExceptionErrorMapper.h"
#include "core/NetworkEnumStrings.h"
#include "core/NetworkErrorUtils.h"
#include "core/QueueBudget.h"
#include "runtime/NetworkRuntimeAccess.h"
#include "runtime/RuntimeCore.h"
#include "tcp/TcpServerSession.h"

#include <asio/bind_executor.hpp>
#include <asio/error.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/steady_timer.hpp>
#include <asio/strand.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ucf::utilities::network {

namespace {

using asio::ip::tcp;
using namespace std::chrono_literals;

[[nodiscard]] bool isRecoverableAcceptError(const NetworkError& error) noexcept
{
    return error.code == NetworkErrorCode::ConnectionAborted
        || error.code == NetworkErrorCode::ConnectionReset
        || error.code == NetworkErrorCode::WouldBlock
        || error.code == NetworkErrorCode::ResourceExhausted;
}

[[nodiscard]] TcpServerOptions validateOptions(TcpServerOptions options)
{
    if (options.listenBacklog == 0
        || options.listenBacklog > static_cast<std::size_t>(std::numeric_limits<int>::max())
        || options.maxConnections == 0
        || options.readBufferSize == 0
        || options.readBufferSize > ByteBuffer{}.max_size()
        || options.maxQueuedWriteBytesPerConnection == 0
        || options.maxTotalQueuedWriteBytes == 0)
    {
        throw std::invalid_argument{"TcpServer limits or read buffer size are invalid"};
    }
    return options;
}

} // namespace

class TcpServer::Impl final
    : public detail::RuntimeParticipant
    , public std::enable_shared_from_this<TcpServer::Impl>
{
public:
    Impl(NetworkRuntime& runtime, TcpServerOptions options, TcpServerHandlers handlers)
        : mCore{detail::NetworkRuntimeAccess::core(runtime)}
        , mControlStrand{asio::make_strand(mCore->executor())}
        , mEventStrand{asio::make_strand(mCore->executor())}
        , mResolver{mControlStrand}
        , mAcceptor{mControlStrand}
        , mAcceptRetryTimer{mControlStrand}
        , mOptions{validateOptions(options)}
        , mHandlers{std::move(handlers)}
        , mTotalWriteBudget{std::make_shared<detail::QueueBudget>(mOptions.maxTotalQueuedWriteBytes)}
    {
    }

    [[nodiscard]] NetworkResult<void> start(Endpoint endpoint)
    {
        std::scoped_lock lock{mCommandMutex};
        if (!mCore->isRunning())
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Listen,
                "NetworkRuntime is not accepting new work"));
        }
        if (mState.load(std::memory_order_acquire) != TcpServerState::Idle)
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Listen,
                "TCP server already has an active lifecycle"));
        }

        const std::uint64_t generation = mGeneration.fetch_add(1, std::memory_order_acq_rel) + 1;
        mState.store(TcpServerState::Starting, std::memory_order_release);
        try
        {
            const auto activityId = mCore->startActivity(
                shared_from_this(),
                mControlStrand,
                [self = shared_from_this(), endpoint = std::move(endpoint), generation](detail::RuntimeActivityId id) mutable {
                    self->startActor(std::move(endpoint), generation, id);
                });
            if (!activityId)
            {
                mState.store(TcpServerState::Idle, std::memory_order_release);
                return NetworkResult<void>::failure(detail::makeError(
                    NetworkErrorCode::InvalidState,
                    NetworkOperation::Listen,
                    "NetworkRuntime stopped before server start was accepted"));
            }
            mActivityId = activityId.value();
        }
        catch (const std::exception& exception)
        {
            mState.store(TcpServerState::Idle, std::memory_order_release);
            return NetworkResult<void>::failure(makeExceptionError(exception, NetworkOperation::Listen));
        }
        catch (...)
        {
            mState.store(TcpServerState::Idle, std::memory_order_release);
            return NetworkResult<void>::failure(makeUnknownExceptionError(NetworkOperation::Listen));
        }

        NETWORK_TRANSPORT_LOG_INFO("TcpServer", "Start accepted; generation=" << generation);
        return NetworkResult<void>::success();
    }

    [[nodiscard]] NetworkResult<void> send(TcpConnectionId connectionId, ByteBuffer data)
    {
        if (data.empty())
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidArgument,
                NetworkOperation::Write,
                "TCP server cannot send an empty buffer"));
        }
        if (mState.load(std::memory_order_acquire) != TcpServerState::Listening)
        {
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Write,
                "TCP server is not listening"));
        }
        std::shared_ptr<TcpServerSession> session;
        {
            std::scoped_lock lock{mSessionsMutex};
            const auto iterator = mSessions.find(connectionId);
            if (iterator == mSessions.end())
            {
                return NetworkResult<void>::failure(detail::makeError(
                    NetworkErrorCode::NotConnected,
                    NetworkOperation::Write,
                    "Unknown TCP connection id"));
            }
            session = iterator->second;
        }
        return session->send(std::move(data));
    }

    void disconnect(TcpConnectionId connectionId) noexcept
    {
        std::shared_ptr<TcpServerSession> session;
        {
            std::scoped_lock lock{mSessionsMutex};
            const auto iterator = mSessions.find(connectionId);
            if (iterator == mSessions.end())
            {
                return;
            }
            session = iterator->second;
        }
        session->close(CloseOrigin::Local);
    }

    void stop() noexcept
    {
        requestStop(TransportCloseInfo{CloseOrigin::Local, std::nullopt, 0});
    }

    void dispose() noexcept
    {
        mCallbacksEnabled.store(false, std::memory_order_release);
        stop();
    }

    [[nodiscard]] TcpServerState state() const noexcept
    {
        return mState.load(std::memory_order_acquire);
    }

    [[nodiscard]] std::size_t connectionCount() const noexcept
    {
        return mConnectionCount.load(std::memory_order_acquire);
    }

    void requestStopFromRuntime() noexcept override
    {
        requestStop(TransportCloseInfo{CloseOrigin::RuntimeShutdown, std::nullopt, 0});
    }

private:
    [[nodiscard]] bool isGeneration(std::uint64_t generation) const noexcept
    {
        return mGeneration.load(std::memory_order_acquire) == generation;
    }

    [[nodiscard]] bool isState(std::uint64_t generation, TcpServerState state) const noexcept
    {
        return isGeneration(generation) && mState.load(std::memory_order_acquire) == state;
    }

    void startActor(
        Endpoint endpoint,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (!isGeneration(generation))
        {
            return;
        }
        // Initialize actor-owned lifecycle state even when stop() synchronously
        // claimed the lifecycle before this first command ran. Otherwise an
        // immediate stop after a restart could inherit mStopEventPosted from
        // the previous generation and suppress onStopped/finishActivity.
        mStopEventPosted = false;
        mDiscardedWriteBytes = 0;
        mPendingStopInfo = {};
        if (!isState(generation, TcpServerState::Starting))
        {
            return;
        }

        if (endpoint.host.empty())
        {
            beginListen(tcp::endpoint{tcp::v4(), endpoint.port}, generation, activityId);
            return;
        }

        asio::error_code addressError;
        const asio::ip::address address = asio::ip::make_address(endpoint.host, addressError);
        if (!addressError)
        {
            beginListen(tcp::endpoint{address, endpoint.port}, generation, activityId);
            return;
        }

        try
        {
            mResolver.async_resolve(
                endpoint.host,
                std::to_string(endpoint.port),
                tcp::resolver::passive,
                [self = shared_from_this(), generation, activityId](
                    const asio::error_code& error,
                    tcp::resolver::results_type results) mutable {
                    self->handleResolve(error, std::move(results), generation, activityId);
                });
        }
        catch (const std::exception& exception)
        {
            failLifecycle(generation, activityId, makeExceptionError(exception, NetworkOperation::Resolve));
        }
        catch (...)
        {
            failLifecycle(generation, activityId, makeUnknownExceptionError(NetworkOperation::Resolve));
        }
    }

    void handleResolve(
        const asio::error_code& error,
        tcp::resolver::results_type results,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (!isState(generation, TcpServerState::Starting))
        {
            return;
        }
        if (error)
        {
            if (error != asio::error::operation_aborted)
            {
                failLifecycle(generation, activityId, makeNetworkError(error, NetworkOperation::Resolve));
            }
            return;
        }
        if (results.empty())
        {
            failLifecycle(
                generation,
                activityId,
                detail::makeError(
                    NetworkErrorCode::NameResolutionFailed,
                    NetworkOperation::Resolve,
                    "TCP bind endpoint resolved to no addresses"));
            return;
        }
        beginListen(results.begin()->endpoint(), generation, activityId);
    }

    void beginListen(
        const tcp::endpoint& endpoint,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (!isState(generation, TcpServerState::Starting))
        {
            return;
        }

        asio::error_code error;
        mAcceptor.open(endpoint.protocol(), error);
        if (!error)
        {
            mAcceptor.set_option(tcp::acceptor::reuse_address{mOptions.reuseAddress}, error);
        }
        if (!error)
        {
            mAcceptor.bind(endpoint, error);
        }
        if (!error)
        {
            mAcceptor.listen(static_cast<int>(mOptions.listenBacklog), error);
        }
        if (error)
        {
            failLifecycle(generation, activityId, makeNetworkError(error, NetworkOperation::Listen));
            return;
        }

        const tcp::endpoint localEndpoint = mAcceptor.local_endpoint(error);
        if (error)
        {
            failLifecycle(generation, activityId, makeNetworkError(error, NetworkOperation::Bind));
            return;
        }
        auto publicEndpoint = detail::toPublicEndpoint(localEndpoint, NetworkOperation::Bind);
        if (!publicEndpoint)
        {
            failLifecycle(generation, activityId, publicEndpoint.error());
            return;
        }

        {
            std::scoped_lock lock{mCommandMutex};
            if (!isState(generation, TcpServerState::Starting))
            {
                return;
            }
            mState.store(TcpServerState::Listening, std::memory_order_release);
        }

        const Endpoint boundEndpoint = std::move(publicEndpoint).value();
        NETWORK_TRANSPORT_LOG_INFO(
            "TcpServer",
            "Listening on " << boundEndpoint.host << ":" << boundEndpoint.port
                << "; generation=" << generation);
        dispatchListening(boundEndpoint, generation);
        beginAccept(generation, activityId);
    }

    void beginAccept(std::uint64_t generation, detail::RuntimeActivityId activityId) noexcept
    {
        if (!isState(generation, TcpServerState::Listening)
            || mAcceptPending
            || mConnectionCount.load(std::memory_order_acquire) >= mOptions.maxConnections)
        {
            return;
        }

        mAcceptPending = true;
        mAcceptPendingGeneration = generation;
        try
        {
            auto sessionStrand = asio::make_strand(mCore->executor());
            mAcceptor.async_accept(
                sessionStrand,
                asio::bind_executor(
                    mControlStrand,
                    [self = shared_from_this(), sessionStrand, generation, activityId](const asio::error_code& error, auto socket) mutable {
                        self->handleAccept(error, std::move(socket), std::move(sessionStrand), generation, activityId);
                    }));
        }
        catch (const std::exception& exception)
        {
            mAcceptPending = false;
            mAcceptPendingGeneration = 0;
            failLifecycle(generation, activityId, makeExceptionError(exception, NetworkOperation::Accept));
        }
        catch (...)
        {
            mAcceptPending = false;
            mAcceptPendingGeneration = 0;
            failLifecycle(generation, activityId, makeUnknownExceptionError(NetworkOperation::Accept));
        }
    }

    template<typename Socket>
    void handleAccept(
        const asio::error_code& error,
        Socket socket,
        asio::strand<asio::any_io_executor> sessionStrand,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (mAcceptPendingGeneration == generation)
        {
            mAcceptPending = false;
            mAcceptPendingGeneration = 0;
        }
        if (!isState(generation, TcpServerState::Listening))
        {
            asio::error_code ignoredError;
            socket.close(ignoredError);
            return;
        }
        if (error)
        {
            handleAcceptError(error, generation, activityId);
            return;
        }

        asio::error_code optionError;
        socket.set_option(tcp::no_delay{mOptions.noDelay}, optionError);
        if (!optionError)
        {
            socket.set_option(asio::socket_base::keep_alive{mOptions.keepAlive}, optionError);
        }
        if (optionError)
        {
            asio::error_code ignoredError;
            socket.close(ignoredError);
            dispatchServerError(makeNetworkError(optionError, NetworkOperation::Accept), generation);
            beginAccept(generation, activityId);
            return;
        }

        const tcp::endpoint local = socket.local_endpoint(optionError);
        if (optionError)
        {
            const NetworkError endpointError = makeNetworkError(optionError, NetworkOperation::Accept);
            asio::error_code ignoredError;
            socket.close(ignoredError);
            dispatchServerError(endpointError, generation);
            beginAccept(generation, activityId);
            return;
        }
        const tcp::endpoint remote = socket.remote_endpoint(optionError);
        if (optionError)
        {
            const NetworkError endpointError = makeNetworkError(optionError, NetworkOperation::Accept);
            asio::error_code ignoredError;
            socket.close(ignoredError);
            dispatchServerError(endpointError, generation);
            beginAccept(generation, activityId);
            return;
        }

        auto publicLocal = detail::toPublicEndpoint(local, NetworkOperation::Accept);
        auto publicRemote = detail::toPublicEndpoint(remote, NetworkOperation::Accept);
        if (!publicLocal || !publicRemote)
        {
            const NetworkError endpointError = !publicLocal ? publicLocal.error() : publicRemote.error();
            asio::error_code ignoredError;
            socket.close(ignoredError);
            dispatchServerError(endpointError, generation);
            beginAccept(generation, activityId);
            return;
        }

        TcpConnectionId connectionId = mNextConnectionId.fetch_add(1, std::memory_order_relaxed);
        if (connectionId == 0)
        {
            connectionId = mNextConnectionId.fetch_add(1, std::memory_order_relaxed);
        }
        std::shared_ptr<TcpServerSession> session;
        try
        {
            std::weak_ptr<Impl> weakSelf = shared_from_this();
            TcpServerSessionHandlers handlers;
            handlers.onReadReady = [weakSelf](
                                             const std::shared_ptr<TcpServerSession>& source,
                                             const std::shared_ptr<ByteBuffer>& buffer,
                                             std::size_t byteCount,
                                             std::optional<NetworkError> terminalError,
                                             std::uint64_t readGeneration) {
                if (const auto self = weakSelf.lock())
                {
                    return self->dispatchData(
                        source,
                        buffer,
                        byteCount,
                        std::move(terminalError),
                        readGeneration);
                }
                return false;
            };
            handlers.onClosed = [weakSelf](
                                    const std::shared_ptr<TcpServerSession>& source,
                                    TransportCloseInfo closeInfo) {
                if (const auto self = weakSelf.lock())
                {
                    return self->dispatchSessionClosed(source, std::move(closeInfo));
                }
                return false;
            };

            session = std::make_shared<TcpServerSession>(
                mCore,
                std::move(sessionStrand),
                tcp::socket{std::move(socket)},
                connectionId,
                mOptions.readBufferSize,
                mOptions.maxQueuedWriteBytesPerConnection,
                mTotalWriteBudget,
                std::move(handlers));
            {
                std::scoped_lock lock{mSessionsMutex};
                mSessions.emplace(connectionId, session);
            }
        }
        catch (const std::exception& exception)
        {
            if (session)
            {
                session->close(CloseOrigin::Error, makeExceptionError(exception, NetworkOperation::Accept));
            }
            dispatchServerError(makeExceptionError(exception, NetworkOperation::Accept), generation);
            beginAccept(generation, activityId);
            return;
        }
        catch (...)
        {
            if (session)
            {
                session->close(CloseOrigin::Error, makeUnknownExceptionError(NetworkOperation::Accept));
            }
            dispatchServerError(makeUnknownExceptionError(NetworkOperation::Accept), generation);
            beginAccept(generation, activityId);
            return;
        }

        mConnectionCount.fetch_add(1, std::memory_order_release);
        TcpConnectionInfo connectionInfo{
            connectionId,
            std::move(publicLocal).value(),
            std::move(publicRemote).value()};
        NETWORK_TRANSPORT_LOG_INFO(
            "TcpServer",
            "Accepted connection id=" << connectionId << " from "
                << connectionInfo.remoteEndpoint.host << ":" << connectionInfo.remoteEndpoint.port);
        dispatchConnected(session, std::move(connectionInfo));
        beginAccept(generation, activityId);
    }

    void handleAcceptError(
        const asio::error_code& error,
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        if (error == asio::error::operation_aborted)
        {
            return;
        }
        NetworkError networkError = makeNetworkError(error, NetworkOperation::Accept);
        if (!isRecoverableAcceptError(networkError))
        {
            failLifecycle(generation, activityId, std::move(networkError));
            return;
        }

        dispatchServerError(networkError, generation);
        try
        {
            mAcceptRetryTimer.expires_after(100ms);
            mAcceptRetryTimer.async_wait([self = shared_from_this(), generation, activityId](const asio::error_code& timerError) {
                if (!timerError)
                {
                    self->beginAccept(generation, activityId);
                }
                else if (timerError != asio::error::operation_aborted)
                {
                    self->failLifecycle(
                        generation,
                        activityId,
                        makeNetworkError(timerError, NetworkOperation::Accept));
                }
            });
        }
        catch (const std::exception& exception)
        {
            failLifecycle(generation, activityId, makeExceptionError(exception, NetworkOperation::Accept));
        }
        catch (...)
        {
            failLifecycle(generation, activityId, makeUnknownExceptionError(NetworkOperation::Accept));
        }
    }

    void dispatchListening(Endpoint endpoint, std::uint64_t generation) noexcept
    {
        static_cast<void>(postEvent([self = shared_from_this(), endpoint = std::move(endpoint), generation] {
            if (!self->isGeneration(generation))
            {
                return;
            }
            if (self->mCallbacksEnabled.load(std::memory_order_acquire)
                && !detail::invokeCallback("TcpServer", "onListening", self->mHandlers.onListening, endpoint))
            {
                self->requestStop(TransportCloseInfo{
                    CloseOrigin::Error,
                    detail::makeCallbackError("onListening"),
                    0});
            }
        }));
    }

    void dispatchConnected(
        std::shared_ptr<TcpServerSession> session,
        TcpConnectionInfo connectionInfo) noexcept
    {
        if (!postEvent([self = shared_from_this(), session, connectionInfo = std::move(connectionInfo)]() mutable {
                bool callbackSucceeded = true;
                if (self->mCallbacksEnabled.load(std::memory_order_acquire))
                {
                    callbackSucceeded = detail::invokeCallback(
                        "TcpServer",
                        "onConnected",
                        self->mHandlers.onConnected,
                        connectionInfo);
                }
                if (!callbackSucceeded)
                {
                    session->close(CloseOrigin::Error, detail::makeCallbackError("onConnected"));
                    return;
                }
                if (!session->start())
                {
                    session->close(
                        CloseOrigin::Error,
                        detail::makeError(NetworkErrorCode::InvalidState, NetworkOperation::Read, "Could not start accepted TCP session"));
                }
            }))
        {
            session->close(
                CloseOrigin::Error,
                detail::makeError(NetworkErrorCode::InvalidState, NetworkOperation::Callback, "Could not dispatch onConnected"));
        }
    }

    [[nodiscard]] bool dispatchData(
        const std::shared_ptr<TcpServerSession>& session,
        const std::shared_ptr<ByteBuffer>& buffer,
        std::size_t byteCount,
        std::optional<NetworkError> terminalError,
        std::uint64_t readGeneration) noexcept
    {
        return postEvent(
            [self = shared_from_this(), session, buffer, byteCount, terminalError = std::move(terminalError), readGeneration]() mutable {
                bool callbackSucceeded = true;
                if (self->mCallbacksEnabled.load(std::memory_order_acquire))
                {
                    callbackSucceeded = detail::invokeCallback(
                        "TcpServer",
                        "onDataReceived",
                        self->mHandlers.onDataReceived,
                        session->id(),
                        ByteView{buffer->data(), byteCount});
                }
                if (!callbackSucceeded)
                {
                    session->close(CloseOrigin::Error, detail::makeCallbackError("onDataReceived"));
                    return;
                }
                session->resumeAfterRead(buffer, std::move(terminalError), readGeneration);
            });
    }

    [[nodiscard]] bool dispatchSessionClosed(
        const std::shared_ptr<TcpServerSession>& session,
        TransportCloseInfo closeInfo) noexcept
    {
        try
        {
            return mCore->postInternal(
                mControlStrand,
                [self = shared_from_this(), session, closeInfo = std::move(closeInfo)]() mutable {
                    self->handleSessionClosed(session, std::move(closeInfo));
                });
        }
        catch (...)
        {
            return false;
        }
    }

    void handleSessionClosed(
        const std::shared_ptr<TcpServerSession>& session,
        TransportCloseInfo closeInfo) noexcept
    {
        bool removed{false};
        {
            std::scoped_lock lock{mSessionsMutex};
            const auto iterator = mSessions.find(session->id());
            if (iterator != mSessions.end() && iterator->second == session)
            {
                mSessions.erase(iterator);
                removed = true;
            }
        }
        if (!removed)
        {
            return;
        }

        mConnectionCount.fetch_sub(1, std::memory_order_release);
        if (closeInfo.discardedWriteBytes > std::numeric_limits<std::size_t>::max() - mDiscardedWriteBytes)
        {
            mDiscardedWriteBytes = std::numeric_limits<std::size_t>::max();
        }
        else
        {
            mDiscardedWriteBytes += closeInfo.discardedWriteBytes;
        }
        const TcpConnectionId connectionId = session->id();
        static_cast<void>(postEvent([self = shared_from_this(), connectionId, closeInfo = std::move(closeInfo)]() mutable {
            if (self->mCallbacksEnabled.load(std::memory_order_acquire))
            {
                static_cast<void>(detail::invokeCallback(
                    "TcpServer",
                    "onConnectionClosed",
                    self->mHandlers.onConnectionClosed,
                    connectionId,
                    closeInfo));
            }
        }));

        const TcpServerState currentState = mState.load(std::memory_order_acquire);
        if (currentState == TcpServerState::Listening)
        {
            detail::RuntimeActivityId activityId{0};
            {
                std::scoped_lock lock{mCommandMutex};
                activityId = mActivityId;
            }
            beginAccept(mGeneration.load(std::memory_order_acquire), activityId);
        }
        else if (currentState == TcpServerState::Stopping && connectionCount() == 0)
        {
            finishStop(
                mGeneration.load(std::memory_order_acquire),
                currentActivityId(),
                std::move(mPendingStopInfo));
        }
    }

    void dispatchServerError(NetworkError error, std::uint64_t generation) noexcept
    {
        NETWORK_TRANSPORT_LOG_WARN(
            "TcpServer",
            "Recoverable listener error; operation=" << detail::toString(error.operation)
                << "; code=" << detail::toString(error.code)
                << "; cause=" << error.diagnostic);
        static_cast<void>(postEvent([self = shared_from_this(), error = std::move(error), generation]() mutable {
            if (!self->isGeneration(generation))
            {
                return;
            }
            if (self->mCallbacksEnabled.load(std::memory_order_acquire)
                && !detail::invokeCallback("TcpServer", "onServerError", self->mHandlers.onServerError, error))
            {
                self->requestStop(TransportCloseInfo{
                    CloseOrigin::Error,
                    detail::makeCallbackError("onServerError"),
                    0});
            }
        }));
    }

    template<typename Event>
    [[nodiscard]] bool postEvent(Event&& event) noexcept
    {
        try
        {
            std::function<void()> erasedEvent{std::forward<Event>(event)};
            return mCore->postInternal(mEventStrand, std::move(erasedEvent));
        }
        catch (...)
        {
            return false;
        }
    }

    void failLifecycle(
        std::uint64_t generation,
        detail::RuntimeActivityId activityId,
        NetworkError error) noexcept
    {
        if (!isGeneration(generation))
        {
            return;
        }
        NETWORK_TRANSPORT_LOG_ERROR(
            "TcpServer",
            "Listener lifecycle failed; operation=" << detail::toString(error.operation)
                << "; code=" << detail::toString(error.code)
                << "; cause=" << error.diagnostic);
        {
            std::scoped_lock lock{mCommandMutex};
            const TcpServerState currentState = mState.load(std::memory_order_acquire);
            if (!isGeneration(generation)
                || (currentState != TcpServerState::Starting
                    && currentState != TcpServerState::Listening))
            {
                return;
            }
            mState.store(TcpServerState::Stopping, std::memory_order_release);
        }
        stopActor(
            generation,
            activityId,
            TransportCloseInfo{CloseOrigin::Error, std::move(error), 0});
    }

    void requestStop(TransportCloseInfo closeInfo) noexcept
    {
        std::uint64_t generation{0};
        detail::RuntimeActivityId activityId{0};
        {
            std::scoped_lock lock{mCommandMutex};
            const TcpServerState currentState = mState.load(std::memory_order_acquire);
            if (currentState == TcpServerState::Idle || currentState == TcpServerState::Stopping)
            {
                return;
            }
            generation = mGeneration.load(std::memory_order_acquire);
            activityId = mActivityId;
            mState.store(TcpServerState::Stopping, std::memory_order_release);
        }

        try
        {
            if (mCore->postInternal(
                    mControlStrand,
                    [self = shared_from_this(), generation, activityId, closeInfo = std::move(closeInfo)]() mutable {
                        self->stopActor(generation, activityId, std::move(closeInfo));
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

    void stopActor(
        std::uint64_t generation,
        detail::RuntimeActivityId activityId,
        TransportCloseInfo closeInfo) noexcept
    {
        if (!isState(generation, TcpServerState::Stopping))
        {
            return;
        }
        try
        {
            mPendingStopInfo = closeInfo;
        }
        catch (...)
        {
            mPendingStopInfo = TransportCloseInfo{closeInfo.origin, std::nullopt, 0};
        }

        asio::error_code ignoredError;
        try
        {
            mResolver.cancel();
        }
        catch (...)
        {
        }
        try
        {
            static_cast<void>(mAcceptRetryTimer.cancel());
        }
        catch (...)
        {
        }
        mAcceptor.cancel(ignoredError);
        mAcceptor.close(ignoredError);
        mAcceptPending = false;
        mAcceptPendingGeneration = 0;

        std::vector<std::shared_ptr<TcpServerSession>> sessions;
        try
        {
            std::scoped_lock lock{mSessionsMutex};
            sessions.reserve(mSessions.size());
            for (const auto& [id, session] : mSessions)
            {
                static_cast<void>(id);
                sessions.emplace_back(session);
            }
        }
        catch (...)
        {
            // Closing via the map below still preserves correctness if the
            // temporary snapshot allocation fails.
            std::scoped_lock lock{mSessionsMutex};
            for (const auto& [id, session] : mSessions)
            {
                static_cast<void>(id);
                try
                {
                    session->close(closeInfo.origin, closeInfo.cause);
                }
                catch (...)
                {
                    session->close(closeInfo.origin);
                }
            }
            return;
        }

        for (const auto& session : sessions)
        {
            try
            {
                session->close(closeInfo.origin, closeInfo.cause);
            }
            catch (...)
            {
                session->close(closeInfo.origin);
            }
        }
        if (sessions.empty())
        {
            finishStop(generation, activityId, std::move(closeInfo));
        }
    }

    void finishStop(
        std::uint64_t generation,
        detail::RuntimeActivityId activityId,
        TransportCloseInfo closeInfo) noexcept
    {
        if (!isState(generation, TcpServerState::Stopping) || mStopEventPosted)
        {
            return;
        }
        mStopEventPosted = true;
        closeInfo.discardedWriteBytes = mDiscardedWriteBytes;

        NETWORK_TRANSPORT_LOG_INFO(
            "TcpServer",
            "Stopped; generation=" << generation
                << "; origin=" << detail::toString(closeInfo.origin)
                << "; discardedWriteBytes=" << closeInfo.discardedWriteBytes);
        if (!postEvent([self = shared_from_this(), closeInfo = std::move(closeInfo), generation, activityId]() mutable {
                bool staleLifecycle{false};
                {
                    std::scoped_lock lock{self->mCommandMutex};
                    if (!self->isState(generation, TcpServerState::Stopping))
                    {
                        staleLifecycle = true;
                    }
                    else
                    {
                        if (self->mActivityId == activityId)
                        {
                            self->mActivityId = 0;
                        }
                        self->mState.store(TcpServerState::Idle, std::memory_order_release);
                    }
                }
                if (staleLifecycle)
                {
                    self->mCore->finishActivity(activityId);
                    return;
                }
                if (self->mCallbacksEnabled.load(std::memory_order_acquire))
                {
                    static_cast<void>(detail::invokeCallback(
                        "TcpServer",
                        "onStopped",
                        self->mHandlers.onStopped,
                        closeInfo));
                }
                self->mCore->finishActivity(activityId);
            }))
        {
            {
                std::scoped_lock lock{mCommandMutex};
                if (isState(generation, TcpServerState::Stopping))
                {
                    if (mActivityId == activityId)
                    {
                        mActivityId = 0;
                    }
                    mState.store(TcpServerState::Idle, std::memory_order_release);
                }
            }
            mCore->finishActivity(activityId);
        }
    }

    [[nodiscard]] detail::RuntimeActivityId currentActivityId() const noexcept
    {
        std::scoped_lock lock{mCommandMutex};
        return mActivityId;
    }

    void abandonLifecycle(
        std::uint64_t generation,
        detail::RuntimeActivityId activityId) noexcept
    {
        {
            std::scoped_lock lock{mCommandMutex};
            if (isGeneration(generation))
            {
                if (mActivityId == activityId)
                {
                    mActivityId = 0;
                }
                mState.store(TcpServerState::Idle, std::memory_order_release);
            }
        }
        {
            std::scoped_lock lock{mSessionsMutex};
            mSessions.clear();
        }
        mConnectionCount.store(0, std::memory_order_release);
        mCore->finishActivity(activityId);
    }

private:
    std::shared_ptr<detail::RuntimeCore> mCore;
    asio::strand<asio::any_io_executor> mControlStrand;
    asio::strand<asio::any_io_executor> mEventStrand;
    tcp::resolver mResolver;
    tcp::acceptor mAcceptor;
    asio::steady_timer mAcceptRetryTimer;
    TcpServerOptions mOptions;
    TcpServerHandlers mHandlers;
    std::shared_ptr<detail::QueueBudget> mTotalWriteBudget;

    std::atomic<TcpServerState> mState{TcpServerState::Idle};
    std::atomic<std::uint64_t> mGeneration{0};
    std::atomic<TcpConnectionId> mNextConnectionId{1};
    std::atomic<std::size_t> mConnectionCount{0};
    std::atomic<bool> mCallbacksEnabled{true};
    mutable std::mutex mCommandMutex;
    detail::RuntimeActivityId mActivityId{0};

    mutable std::mutex mSessionsMutex;
    std::unordered_map<TcpConnectionId, std::shared_ptr<TcpServerSession>> mSessions;

    bool mAcceptPending{false};
    std::uint64_t mAcceptPendingGeneration{0};
    bool mStopEventPosted{false};
    std::size_t mDiscardedWriteBytes{0};
    TransportCloseInfo mPendingStopInfo;
};

TcpServer::TcpServer(NetworkRuntime& runtime, TcpServerHandlers handlers)
    : TcpServer{runtime, TcpServerOptions{}, std::move(handlers)}
{
}

TcpServer::TcpServer(NetworkRuntime& runtime, TcpServerOptions options, TcpServerHandlers handlers)
    : mImpl{std::make_shared<Impl>(runtime, options, std::move(handlers))}
{
}

TcpServer::~TcpServer()
{
    mImpl->dispose();
}

NetworkResult<void> TcpServer::start(Endpoint bindEndpoint)
{
    return mImpl->start(std::move(bindEndpoint));
}

NetworkResult<void> TcpServer::send(TcpConnectionId connectionId, ByteBuffer data)
{
    return mImpl->send(connectionId, std::move(data));
}

void TcpServer::disconnect(TcpConnectionId connectionId) noexcept
{
    mImpl->disconnect(connectionId);
}

void TcpServer::stop() noexcept
{
    mImpl->stop();
}

TcpServerState TcpServer::state() const noexcept
{
    return mImpl->state();
}

std::size_t TcpServer::connectionCount() const noexcept
{
    return mImpl->connectionCount();
}

} // namespace ucf::utilities::network
