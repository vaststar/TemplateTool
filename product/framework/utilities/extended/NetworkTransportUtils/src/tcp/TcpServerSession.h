#pragma once

#include <ucf/utilities/NetworkTransportUtils/core/ByteBuffer.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkResult.h>
#include <ucf/utilities/NetworkTransportUtils/core/TransportCloseInfo.h>
#include <ucf/utilities/NetworkTransportUtils/tcp/TcpServer.h>

#include "core/QueueBudget.h"
#include "runtime/RuntimeCore.h"

#include <asio/any_io_executor.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/strand.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>

namespace ucf::utilities::network {

class TcpServerSession;

struct TcpServerSessionHandlers final
{
    // Returning true means ownership of the read buffer was handed to the
    // server event strand. That strand must eventually call resumeAfterRead().
    std::function<bool(
        const std::shared_ptr<TcpServerSession>&,
        const std::shared_ptr<ByteBuffer>&,
        std::size_t,
        std::optional<NetworkError>,
        std::uint64_t)> onReadReady;
    std::function<bool(
        const std::shared_ptr<TcpServerSession>&,
        TransportCloseInfo)> onClosed;
};

/// Internal actor for one accepted socket. Every socket initiation and handler
/// runs through mStrand; the server's event strand never touches the socket.
class TcpServerSession final : public std::enable_shared_from_this<TcpServerSession>
{
public:
    TcpServerSession(
        std::shared_ptr<detail::RuntimeCore> core,
        asio::strand<asio::any_io_executor> strand,
        asio::ip::tcp::socket socket,
        TcpConnectionId connectionId,
        std::size_t readBufferSize,
        std::size_t maxQueuedWriteBytes,
        std::shared_ptr<detail::QueueBudget> totalWriteBudget,
        TcpServerSessionHandlers handlers);

    TcpServerSession(const TcpServerSession&) = delete;
    TcpServerSession& operator=(const TcpServerSession&) = delete;

    /// Posts the transition from accepted/paused to active. Keeping this as an
    /// actor command makes send() from onConnected causally ordered with start.
    [[nodiscard]] bool start() noexcept;
    [[nodiscard]] NetworkResult<void> send(ByteBuffer data);
    void close(CloseOrigin origin = CloseOrigin::Local, std::optional<NetworkError> cause = std::nullopt) noexcept;

    /// Returns a read buffer from the server event strand. No second read is in
    /// flight until this command reaches the session actor.
    void resumeAfterRead(
        std::shared_ptr<ByteBuffer> buffer,
        std::optional<NetworkError> terminalReadError,
        std::uint64_t generation) noexcept;

    [[nodiscard]] TcpConnectionId id() const noexcept { return mConnectionId; }

private:
    enum class State
    {
        Pending,
        Open,
        Closing,
        Closed
    };

    [[nodiscard]] bool isCurrent(std::uint64_t generation, State expected) const noexcept;
    [[nodiscard]] bool isWritable(std::uint64_t generation) const noexcept;
    void startActor(std::uint64_t generation) noexcept;
    void startRead(const std::shared_ptr<ByteBuffer>& buffer, std::uint64_t generation) noexcept;
    void handleRead(
        const std::shared_ptr<ByteBuffer>& buffer,
        const asio::error_code& error,
        std::size_t byteCount,
        std::uint64_t generation) noexcept;

    void enqueueWrite(std::shared_ptr<ByteBuffer> data, std::uint64_t generation) noexcept;
    void startWrite(std::uint64_t generation) noexcept;
    void handleWrite(
        const asio::error_code& error,
        std::size_t byteCount,
        std::size_t expectedByteCount,
        std::uint64_t generation) noexcept;

    void requestClose(CloseOrigin origin, std::optional<NetworkError> cause) noexcept;
    void finishClose(std::uint64_t generation, TransportCloseInfo closeInfo) noexcept;
    void releaseQueuedWriteBytes(std::uint64_t generation, std::size_t byteCount) noexcept;

private:
    std::shared_ptr<detail::RuntimeCore> mCore;
    asio::strand<asio::any_io_executor> mStrand;
    asio::ip::tcp::socket mSocket;
    const TcpConnectionId mConnectionId;
    const std::size_t mReadBufferSize;
    const std::size_t mMaxQueuedWriteBytes;
    std::shared_ptr<detail::QueueBudget> mTotalWriteBudget;
    TcpServerSessionHandlers mHandlers;
    std::deque<std::shared_ptr<ByteBuffer>> mWriteQueue;

    std::atomic<State> mState{State::Pending};
    std::atomic<std::uint64_t> mGeneration{1};
    mutable std::mutex mCommandMutex;
    std::size_t mQueuedWriteBytes{0};
};

} // namespace ucf::utilities::network
