#pragma once

#include <atomic>
#include <mutex>
#include <thread>

#include <ucf/Utilities/TcpChannel/ITcpChannel.h>
#include <ucf/Utilities/TcpChannel/ITcpChannelCallback.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include "SocketHelper.h"

namespace ucf::utilities {

/// Internal lifecycle state for TcpChannel.
enum class ChannelState : int
{
    Idle,       ///< Not listening
    Starting,   ///< startListening() in progress (socket setup)
    Listening,  ///< I/O thread running, accepting connections
    Stopping    ///< stop() in progress
};

/// Concrete implementation of ITcpChannel.
///
/// Runs a select()-based I/O loop on a background thread.
/// Accepts a single client connection and provides bidirectional
/// byte-stream communication via callbacks.
///
/// Thread safety:
///   - startListening() / stop() are guarded by CAS on mChannelState.
///   - stop() from within a callback (I/O thread) is safe — cleanup
///     is deferred to the ioLoop exit path.
///   - mClientSocket is protected by mClientMutex for send() / I/O thread.
class TcpChannel final
    : public virtual NotificationHelper<ITcpChannelCallback>
    , public ITcpChannel
{
public:
    TcpChannel();
    ~TcpChannel() override;

    TcpChannel(const TcpChannel&) = delete;
    TcpChannel& operator=(const TcpChannel&) = delete;
    TcpChannel(TcpChannel&&) = delete;
    TcpChannel& operator=(TcpChannel&&) = delete;

    // ── ITcpChannel ──
    bool startListening(const TcpChannelConfig& config) override;
    void stop() override;
    bool send(const std::string& data) override;
    bool isConnected() const override;
    bool isListening() const override;
    int  listeningPort() const override;

private:
    /// Background I/O loop using select().
    void ioLoop();

    /// Handle a new incoming connection on the server socket.
    void handleNewConnection();

    /// Handle readable data on the client socket.
    void handleClientData();

    /// Close client socket under lock.
    void disconnectClient();

    /// Close both server and client sockets, reset port.
    /// Called from stop() or ioLoop exit (deferred cleanup).
    void cleanupSockets();

    SocketHandle mServerSocket = kInvalidSocket;
    SocketHandle mClientSocket = kInvalidSocket;
    mutable std::mutex mClientMutex;

    std::thread mIoThread;
    std::atomic<ChannelState> mChannelState{ChannelState::Idle};
    std::atomic<bool> mStopRequested{false};
    std::atomic<int>  mPort{0};

    TcpChannelConfig mConfig;

    static constexpr int kSelectTimeoutMs = 50;
    static constexpr int kRecvBufferSize = 8192;
};

} // namespace ucf::utilities
