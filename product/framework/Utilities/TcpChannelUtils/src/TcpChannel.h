#pragma once

#include <atomic>
#include <initializer_list>
#include <mutex>
#include <thread>

#include <ucf/Utilities/TcpChannelUtils/ITcpChannel.h>
#include <ucf/Utilities/TcpChannelUtils/ITcpChannelCallback.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include "SocketHelper.h"

namespace ucf::utilities {

/// Internal lifecycle state for TcpChannel.
enum class ChannelState : int
{
    Idle,        ///< Not listening, never started or re-usable after cleanup
    Starting,    ///< startListening() in progress (socket setup)
    Listening,   ///< I/O thread running, accepting connections
    Stopping,    ///< stop() in progress
    Terminated   ///< Stopped / I/O error exit. Can restart from here.
};

/// Concrete implementation of ITcpChannel.
///
/// Runs a select()-based I/O loop on a background thread.
/// Accepts a single client connection and provides bidirectional
/// byte-stream communication via callbacks.
///
/// Thread safety:
///   - startListening() / stop() are guarded by CAS on mChannelState.
///   - stop() from within a callback (I/O thread) is safe â€” cleanup
///     is deferred to the ioLoop exit path.
///   - mClientSocket is protected by mClientMutex for send() / I/O thread.
class TcpChannel final : public virtual ITcpChannel, public virtual NotificationHelper<ITcpChannelCallback>
{
public:
    TcpChannel();
    ~TcpChannel() override;

    TcpChannel(const TcpChannel&) = delete;
    TcpChannel& operator=(const TcpChannel&) = delete;
    TcpChannel(TcpChannel&&) = delete;
    TcpChannel& operator=(TcpChannel&&) = delete;

    // â”€â”€ ITcpChannel â”€â”€
    bool startListening(const TcpChannelConfig& config) override;
    void stop() override;
    bool send(const std::string& data) override;
    bool isConnected() const override;
    bool isListening() const override;
    int  listeningPort() const override;

private:
    // â”€â”€ State machine â”€â”€

    /// Attempt to transition mChannelState to @p to.
    /// Valid source states are defined internally per target state.
    bool tryTransition(ChannelState to);

    /// CAS helper: try transitioning from any of @p fromStates to @p to.
    bool casFrom(std::initializer_list<ChannelState> fromStates, ChannelState to);

    /// Attempt to acquire stop ownership by transitioning to Stopping.
    bool beginStop();

    /// Finalize a startup failure before Listening was reached.
    void failStart(const std::string& errorMessage);

    /// Read the current channel state (acquire semantics).
    ChannelState currentState() const;

    // â”€â”€ I/O â”€â”€

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
    void finalizeStop();

    SocketHandle mServerSocket = kInvalidSocket;
    SocketHandle mClientSocket = kInvalidSocket;
    mutable std::mutex mClientMutex;

    std::thread mIoThread;
    std::atomic<ChannelState> mChannelState{ChannelState::Idle};
    std::atomic<int>  mPort{0};

    /// Guards mConfig/mServerSocket/mIoThread writes during startListening() setup.
    /// stop() acquires briefly as a sync barrier to ensure setup is complete.
    std::mutex mLifecycleMutex;

    TcpChannelConfig mConfig;

    static constexpr int kSelectTimeoutMs = 50;
    static constexpr int kRecvBufferSize = 8192;
};

} // namespace ucf::utilities
