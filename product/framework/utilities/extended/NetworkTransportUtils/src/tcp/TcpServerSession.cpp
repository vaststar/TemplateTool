#include "tcp/TcpServerSession.h"

#include "NetworkTransportLogger.h"
#include "core/AsioErrorMapper.h"
#include "core/ExceptionErrorMapper.h"
#include "core/NetworkEnumStrings.h"
#include "core/NetworkErrorUtils.h"
#include "tcp/TcpCloseInfoUtils.h"

#include <asio/bind_executor.hpp>
#include <asio/buffer.hpp>
#include <asio/error.hpp>
#include <asio/write.hpp>

#include <algorithm>
#include <exception>
#include <utility>

namespace ucf::utilities::network {

TcpServerSession::TcpServerSession(
    std::shared_ptr<detail::RuntimeCore> core,
    asio::strand<asio::any_io_executor> strand,
    asio::ip::tcp::socket socket,
    TcpConnectionId connectionId,
    std::size_t readBufferSize,
    std::size_t maxQueuedWriteBytes,
    std::shared_ptr<detail::QueueBudget> totalWriteBudget,
    TcpServerSessionHandlers handlers)
    : mCore{std::move(core)}
    , mStrand{std::move(strand)}
    , mSocket{std::move(socket)}
    , mConnectionId{connectionId}
    , mReadBufferSize{readBufferSize}
    , mMaxQueuedWriteBytes{maxQueuedWriteBytes}
    , mTotalWriteBudget{std::move(totalWriteBudget)}
    , mHandlers{std::move(handlers)}
{
}

bool TcpServerSession::start() noexcept
{
    const std::uint64_t generation = mGeneration.load(std::memory_order_acquire);
    try
    {
        return mCore->postInternal(mStrand, [self = shared_from_this(), generation] {
            self->startActor(generation);
        });
    }
    catch (...)
    {
        requestClose(
            CloseOrigin::Error,
            detail::makeError(NetworkErrorCode::ResourceExhausted, NetworkOperation::Read, "Could not schedule TCP session start"));
        return false;
    }
}

NetworkResult<void> TcpServerSession::send(ByteBuffer data)
{
    if (data.empty())
    {
        return NetworkResult<void>::failure(detail::makeError(
            NetworkErrorCode::InvalidArgument,
            NetworkOperation::Write,
            "TCP server cannot send an empty buffer"));
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

    const State currentState = mState.load(std::memory_order_acquire);
    if (currentState != State::Pending && currentState != State::Open)
    {
        return NetworkResult<void>::failure(detail::makeError(
            NetworkErrorCode::NotConnected,
            NetworkOperation::Write,
            "TCP connection is closing or closed"));
    }
    if (byteCount > mMaxQueuedWriteBytes - mQueuedWriteBytes)
    {
        NETWORK_TRANSPORT_LOG_DEBUG(
            "TcpServer",
            "Connection write queue full; id=" << mConnectionId
                << "; requested=" << byteCount << "; queued=" << mQueuedWriteBytes
                << "; limit=" << mMaxQueuedWriteBytes);
        return NetworkResult<void>::failure(detail::makeError(
            NetworkErrorCode::QueueFull,
            NetworkOperation::Write,
            "TCP connection write queue is full"));
    }
    if (!mTotalWriteBudget->tryReserve(byteCount))
    {
        NETWORK_TRANSPORT_LOG_DEBUG(
            "TcpServer",
            "Server write budget full; id=" << mConnectionId
                << "; requested=" << byteCount << "; totalQueued=" << mTotalWriteBudget->used());
        return NetworkResult<void>::failure(detail::makeError(
            NetworkErrorCode::QueueFull,
            NetworkOperation::Write,
            "TCP server total write queue is full"));
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
            mTotalWriteBudget->release(byteCount);
            return NetworkResult<void>::failure(detail::makeError(
                NetworkErrorCode::InvalidState,
                NetworkOperation::Write,
                "NetworkRuntime stopped before write was scheduled"));
        }
    }
    catch (const std::exception& exception)
    {
        mQueuedWriteBytes -= byteCount;
        mTotalWriteBudget->release(byteCount);
        return NetworkResult<void>::failure(makeExceptionError(exception, NetworkOperation::Write));
    }
    catch (...)
    {
        mQueuedWriteBytes -= byteCount;
        mTotalWriteBudget->release(byteCount);
        return NetworkResult<void>::failure(makeUnknownExceptionError(NetworkOperation::Write));
    }
    return NetworkResult<void>::success();
}

void TcpServerSession::close(CloseOrigin origin, std::optional<NetworkError> cause) noexcept
{
    requestClose(origin, std::move(cause));
}

void TcpServerSession::resumeAfterRead(
    std::shared_ptr<ByteBuffer> buffer,
    std::optional<NetworkError> terminalReadError,
    std::uint64_t generation) noexcept
{
    try
    {
        if (!mCore->postInternal(
                mStrand,
                [self = shared_from_this(),
                    buffer = std::move(buffer),
                    terminalReadError = std::move(terminalReadError),
                    generation]() mutable {
                    if (!self->isCurrent(generation, State::Open))
                    {
                        return;
                    }
                    if (terminalReadError)
                    {
                        const auto closeInfo = detail::makeTcpCloseInfo(std::move(terminalReadError.value()));
                        self->requestClose(closeInfo.origin, std::move(closeInfo.cause));
                        return;
                    }
                    self->startRead(buffer, generation);
                }))
        {
            requestClose(
                CloseOrigin::Error,
                detail::makeError(
                    NetworkErrorCode::InvalidState,
                    NetworkOperation::Read,
                    "Runtime stopped before read buffer was returned"));
        }
    }
    catch (...)
    {
        requestClose(
            CloseOrigin::Error,
            detail::makeError(
                NetworkErrorCode::ResourceExhausted,
                NetworkOperation::Read,
                "Could not return TCP read buffer to session actor"));
    }
}

bool TcpServerSession::isCurrent(std::uint64_t generation, State expected) const noexcept
{
    return mGeneration.load(std::memory_order_acquire) == generation
        && mState.load(std::memory_order_acquire) == expected;
}

bool TcpServerSession::isWritable(std::uint64_t generation) const noexcept
{
    if (mGeneration.load(std::memory_order_acquire) != generation)
    {
        return false;
    }
    const State currentState = mState.load(std::memory_order_acquire);
    return currentState == State::Pending || currentState == State::Open;
}

void TcpServerSession::startActor(std::uint64_t generation) noexcept
{
    {
        std::scoped_lock lock{mCommandMutex};
        if (!isCurrent(generation, State::Pending))
        {
            return;
        }
        mState.store(State::Open, std::memory_order_release);
    }

    std::shared_ptr<ByteBuffer> buffer;
    try
    {
        buffer = std::make_shared<ByteBuffer>(mReadBufferSize);
    }
    catch (const std::exception& exception)
    {
        requestClose(CloseOrigin::Error, makeExceptionError(exception, NetworkOperation::Read));
        return;
    }
    catch (...)
    {
        requestClose(CloseOrigin::Error, makeUnknownExceptionError(NetworkOperation::Read));
        return;
    }

    if (!mWriteQueue.empty())
    {
        startWrite(generation);
    }
    startRead(buffer, generation);
}

void TcpServerSession::startRead(const std::shared_ptr<ByteBuffer>& buffer, std::uint64_t generation) noexcept
{
    if (!isCurrent(generation, State::Open))
    {
        return;
    }
    try
    {
        mSocket.async_read_some(
            asio::buffer(*buffer),
            asio::bind_executor(
                mStrand,
                [self = shared_from_this(), buffer, generation](const asio::error_code& error, std::size_t byteCount) {
                    self->handleRead(buffer, error, byteCount, generation);
                }));
    }
    catch (const std::exception& exception)
    {
        requestClose(CloseOrigin::Error, makeExceptionError(exception, NetworkOperation::Read));
    }
    catch (...)
    {
        requestClose(CloseOrigin::Error, makeUnknownExceptionError(NetworkOperation::Read));
    }
}

void TcpServerSession::handleRead(
    const std::shared_ptr<ByteBuffer>& buffer,
    const asio::error_code& error,
    std::size_t byteCount,
    std::uint64_t generation) noexcept
{
    if (!isCurrent(generation, State::Open))
    {
        return;
    }

    std::optional<NetworkError> terminalError;
    if (error)
    {
        terminalError = makeNetworkError(error, NetworkOperation::Read);
    }

    if (byteCount > 0)
    {
        try
        {
            if (mHandlers.onReadReady
                && mHandlers.onReadReady(shared_from_this(), buffer, byteCount, terminalError, generation))
            {
                return;
            }
        }
        catch (...)
        {
        }
        requestClose(
            CloseOrigin::Error,
            detail::makeError(NetworkErrorCode::CallbackFailed, NetworkOperation::Callback, "Could not dispatch TCP server data callback"));
        return;
    }

    if (terminalError)
    {
        if (error != asio::error::operation_aborted)
        {
            const auto closeInfo = detail::makeTcpCloseInfo(std::move(terminalError.value()));
            requestClose(closeInfo.origin, std::move(closeInfo.cause));
        }
        return;
    }
    startRead(buffer, generation);
}

void TcpServerSession::enqueueWrite(std::shared_ptr<ByteBuffer> data, std::uint64_t generation) noexcept
{
    if (!isWritable(generation))
    {
        // Keep same-generation admission accounting intact while a close is
        // pending so onConnectionClosed reports this accepted write as
        // discarded. Closed sessions have already cleared their counters.
        return;
    }

    const bool writeInProgress = !mWriteQueue.empty();
    try
    {
        mWriteQueue.emplace_back(std::move(data));
    }
    catch (const std::exception& exception)
    {
        requestClose(CloseOrigin::Error, makeExceptionError(exception, NetworkOperation::Write));
        return;
    }
    catch (...)
    {
        requestClose(CloseOrigin::Error, makeUnknownExceptionError(NetworkOperation::Write));
        return;
    }

    if (!writeInProgress && mState.load(std::memory_order_acquire) == State::Open)
    {
        startWrite(generation);
    }
}

void TcpServerSession::startWrite(std::uint64_t generation) noexcept
{
    if (!isCurrent(generation, State::Open) || mWriteQueue.empty())
    {
        return;
    }
    const auto data = mWriteQueue.front();
    try
    {
        asio::async_write(
            mSocket,
            asio::buffer(*data),
            asio::bind_executor(
                mStrand,
                [self = shared_from_this(), data, generation](const asio::error_code& error, std::size_t byteCount) {
                    self->handleWrite(error, byteCount, data->size(), generation);
                }));
    }
    catch (const std::exception& exception)
    {
        requestClose(CloseOrigin::Error, makeExceptionError(exception, NetworkOperation::Write));
    }
    catch (...)
    {
        requestClose(CloseOrigin::Error, makeUnknownExceptionError(NetworkOperation::Write));
    }
}

void TcpServerSession::handleWrite(
    const asio::error_code& error,
    std::size_t byteCount,
    std::size_t expectedByteCount,
    std::uint64_t generation) noexcept
{
    if (mGeneration.load(std::memory_order_acquire) != generation)
    {
        return;
    }
    // A failed composed write can still have transferred a prefix. Release the
    // confirmed prefix now so closeInfo reports only genuinely discarded bytes,
    // even if disconnect()/stop() claimed the close before handler delivery.
    releaseQueuedWriteBytes(generation, std::min(byteCount, expectedByteCount));
    if (!isCurrent(generation, State::Open))
    {
        // disconnect()/stop() already selected the terminal reason; let its
        // queued close command finish the actor lifecycle.
        return;
    }
    if (error)
    {
        if (error != asio::error::operation_aborted)
        {
            const auto closeInfo = detail::makeTcpCloseInfo(makeNetworkError(error, NetworkOperation::Write));
            requestClose(closeInfo.origin, std::move(closeInfo.cause));
        }
        return;
    }
    if (byteCount != expectedByteCount)
    {
        requestClose(
            CloseOrigin::Error,
            detail::makeError(
                NetworkErrorCode::SystemError,
                NetworkOperation::Write,
                "TCP async_write completed with an unexpected byte count"));
        return;
    }
    mWriteQueue.pop_front();
    startWrite(generation);
}

void TcpServerSession::requestClose(CloseOrigin origin, std::optional<NetworkError> cause) noexcept
{
    std::uint64_t generation{0};
    {
        std::scoped_lock lock{mCommandMutex};
        const State currentState = mState.load(std::memory_order_acquire);
        if (currentState == State::Closing || currentState == State::Closed)
        {
            return;
        }
        generation = mGeneration.load(std::memory_order_acquire);
        mState.store(State::Closing, std::memory_order_release);
    }

    try
    {
        if (mCore->postInternal(
                mStrand,
                [self = shared_from_this(), generation, closeInfo = TransportCloseInfo{origin, std::move(cause), 0}]() mutable {
                    self->finishClose(generation, std::move(closeInfo));
                }))
        {
            return;
        }
    }
    catch (...)
    {
    }

    // Runtime fault fallback: release admission accounting without touching the
    // socket from the wrong thread. Socket destruction will perform final OS
    // cleanup once outstanding handler ownership is released.
    std::size_t discardedBytes{0};
    {
        std::scoped_lock lock{mCommandMutex};
        discardedBytes = mQueuedWriteBytes;
        mQueuedWriteBytes = 0;
        mState.store(State::Closed, std::memory_order_release);
    }
    mTotalWriteBudget->release(discardedBytes);
}

void TcpServerSession::finishClose(
    std::uint64_t generation,
    TransportCloseInfo closeInfo) noexcept
{
    std::size_t discardedBytes{0};
    {
        std::scoped_lock lock{mCommandMutex};
        if (mGeneration.load(std::memory_order_acquire) != generation
            || mState.load(std::memory_order_acquire) == State::Closed)
        {
            return;
        }
        discardedBytes = mQueuedWriteBytes;
        mQueuedWriteBytes = 0;
        mState.store(State::Closed, std::memory_order_release);
    }
    closeInfo.discardedWriteBytes = discardedBytes;
    mTotalWriteBudget->release(discardedBytes);

    asio::error_code ignoredError;
    mSocket.cancel(ignoredError);
    mSocket.shutdown(asio::ip::tcp::socket::shutdown_both, ignoredError);
    mSocket.close(ignoredError);
    mWriteQueue.clear();

    if (closeInfo.origin == CloseOrigin::Error)
    {
        NETWORK_TRANSPORT_LOG_WARN(
            "TcpServer",
            "Connection id=" << mConnectionId << " failed; cause="
                << (closeInfo.cause ? closeInfo.cause->diagnostic : std::string{"unavailable"})
                << "; code=" << (closeInfo.cause ? detail::toString(closeInfo.cause->code) : "Unknown")
                << "; operation=" << (closeInfo.cause ? detail::toString(closeInfo.cause->operation) : "None")
                << "; discardedWriteBytes=" << closeInfo.discardedWriteBytes);
    }
    else
    {
        NETWORK_TRANSPORT_LOG_INFO(
            "TcpServer",
            "Connection id=" << mConnectionId << " closed; origin=" << detail::toString(closeInfo.origin)
                << "; discardedWriteBytes=" << closeInfo.discardedWriteBytes);
    }
    try
    {
        if (mHandlers.onClosed)
        {
            static_cast<void>(mHandlers.onClosed(shared_from_this(), std::move(closeInfo)));
        }
    }
    catch (...)
    {
        NETWORK_TRANSPORT_LOG_ERROR("TcpServer", "Internal session-close dispatch failed for id=" << mConnectionId);
    }
}

void TcpServerSession::releaseQueuedWriteBytes(
    std::uint64_t generation,
    std::size_t byteCount) noexcept
{
    std::size_t releasedBytes{0};
    {
        std::scoped_lock lock{mCommandMutex};
        if (mGeneration.load(std::memory_order_acquire) == generation)
        {
            releasedBytes = std::min(byteCount, mQueuedWriteBytes);
            mQueuedWriteBytes -= releasedBytes;
        }
    }
    if (releasedBytes > 0)
    {
        mTotalWriteBudget->release(releasedBytes);
    }
}

} // namespace ucf::utilities::network
