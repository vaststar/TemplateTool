#include "TcpChannel.h"
#include "TcpChannelLogger.h"

#include <magic_enum/magic_enum.hpp>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/select.h>
#endif

namespace ucf::utilities {

// ════════════════════════════════════════════════
//  Factory
// ════════════════════════════════════════════════

std::unique_ptr<ITcpChannel> ITcpChannel::create()
{
    return std::make_unique<TcpChannel>();
}

// ════════════════════════════════════════════════
//  State machine
// ════════════════════════════════════════════════

bool TcpChannel::tryTransition(ChannelState to)
{
    switch (to)
    {
    case ChannelState::Starting:
        if (casFrom({ChannelState::Idle, ChannelState::Terminated}, to))
        {
            return true;
        }
        break;

    case ChannelState::Listening:
        if (casFrom({ChannelState::Starting}, to))
        {
            return true;
        }
        break;

    case ChannelState::Stopping:
        if (casFrom({ChannelState::Starting, ChannelState::Listening}, to))
        {
            return true;
        }
        break;

    case ChannelState::Terminated:
        if (casFrom({ChannelState::Starting, ChannelState::Stopping}, to))
        {
            return true;
        }
        break;

    case ChannelState::Idle:
        break;  // Idle is the initial state, no transition back
    }

    TC_LOG_DEBUG("Transition to " << magic_enum::enum_name(to)
                 << " rejected, current=" << magic_enum::enum_name(currentState()));
    return false;
}

bool TcpChannel::casFrom(std::initializer_list<ChannelState> fromStates,
                         ChannelState to)
{
    for (auto from : fromStates)
    {
        auto expected = from;
        if (mChannelState.compare_exchange_strong(expected, to,
                                                  std::memory_order_acq_rel,
                                                  std::memory_order_acquire))
        {
            TC_LOG_DEBUG("State: " << magic_enum::enum_name(from)
                         << " -> " << magic_enum::enum_name(to));
            return true;
        }
    }
    return false;
}

bool TcpChannel::beginStop()
{
    if (!tryTransition(ChannelState::Stopping))
    {
        TC_LOG_DEBUG("Stop skipped, state=" << magic_enum::enum_name(currentState()));
        return false;
    }

    TC_LOG_INFO("Stop begin, state=" << magic_enum::enum_name(currentState()));
    return true;
}

void TcpChannel::failStart(const std::string& errorMessage)
{
    tryTransition(ChannelState::Terminated);
    fireNotification(&ITcpChannelCallback::onError, errorMessage);
}

// ════════════════════════════════════════════════
//  Lifecycle
// ════════════════════════════════════════════════

TcpChannel::TcpChannel()
{
    detail::SocketHelper::initialize();
}

TcpChannel::~TcpChannel()
{
    stop();
    // Ensure the I/O thread is joined even if it exited on its own
    // (e.g. select() error → ioLoop returned without stop() joining it).
    if (mIoThread.joinable())
    {
        mIoThread.join();
    }
    detail::SocketHelper::cleanup();
}

bool TcpChannel::startListening(const TcpChannelConfig& config)
{
    if (!tryTransition(ChannelState::Starting))
    {
        TC_LOG_WARN("startListening() rejected, state=" << magic_enum::enum_name(currentState()));
        return false;
    }

    // Clean up a previous I/O thread that may still be joinable
    // (e.g. after a start → stop → start cycle)
    if (mIoThread.joinable())
    {
        mIoThread.join();
    }

    // Lock scope: protect shared data writes so that stop() cannot read
    // mServerSocket/mIoThread in a partially-written state.
    bool setupFailed = false;
    std::string errorMsg;
    {
        std::lock_guard<std::mutex> lock(mLifecycleMutex);

        mConfig = config;

        // Create server socket
        mServerSocket = detail::SocketHelper::createTcp();
        if (mServerSocket == kInvalidSocket)
        {
            errorMsg = "Failed to create socket: " + detail::SocketHelper::lastError();
            TC_LOG_ERROR(errorMsg);
            setupFailed = true;
        }
        else
        {
            detail::SocketHelper::setReuseAddr(mServerSocket);

            if (!detail::SocketHelper::bindAndListen(mServerSocket, config.listenAddress,
                                                      config.listenPort, config.maxConnections))
            {
                errorMsg = "Failed to bind/listen on port " + std::to_string(config.listenPort)
                           + ": " + detail::SocketHelper::lastError();
                TC_LOG_ERROR(errorMsg);
                detail::SocketHelper::closeSocket(mServerSocket);
                setupFailed = true;
            }
            else
            {
                mPort = detail::SocketHelper::getLocalPort(mServerSocket);

                TC_LOG_INFO("Listening on " << config.listenAddress << ":" << mPort.load());

                // Start I/O thread BEFORE setting Listening —
                // ensures the thread object is valid when stop() tries to join
                mIoThread = std::thread(&TcpChannel::ioLoop, this);
            }
        }
    }
    // Lock released — stop() can now safely access mServerSocket/mIoThread

    if (setupFailed)
    {
        failStart(errorMsg);
        return false;
    }

    if (!tryTransition(ChannelState::Listening))
    {
        // stop() intervened during setup
        TC_LOG_WARN("startListening() interrupted by concurrent stop(), state="
                    << magic_enum::enum_name(currentState()));
        return false;
    }

    return true;
}

void TcpChannel::stop()
{
    if (!beginStop())
    {
        return;
    }

    TC_LOG_INFO("Stopping TcpChannel");

    // Sync barrier: wait for startListening() setup to complete (if it's still
    // inside the locked region writing mServerSocket/mIoThread).
    {
        std::lock_guard<std::mutex> lock(mLifecycleMutex);
    }

    // If called from within a callback on the I/O thread, we cannot
    // join ourselves. The ioLoop exit path will do deferred cleanup.
    if (std::this_thread::get_id() == mIoThread.get_id())
    {
        TC_LOG_INFO("stop() called from I/O thread, cleanup deferred to ioLoop exit");
        return;
    }

    // Join the I/O thread first — it uses both sockets
    if (mIoThread.joinable())
    {
        mIoThread.join();
    }

    // ioLoop deferred path may have already cleaned up and set Terminated
    if (currentState() == ChannelState::Stopping)
    {
        finalizeStop();
    }
    TC_LOG_INFO("TcpChannel stopped");
}

// ════════════════════════════════════════════════
//  Send
// ════════════════════════════════════════════════

bool TcpChannel::send(const std::string& data)
{
    std::lock_guard<std::mutex> lock(mClientMutex);
    if (mClientSocket == kInvalidSocket)
    {
        return false;
    }
    return detail::SocketHelper::sendAll(mClientSocket, data.data(), static_cast<int>(data.size()));
}

// ════════════════════════════════════════════════
//  Queries
// ════════════════════════════════════════════════

bool TcpChannel::isConnected() const
{
    std::lock_guard<std::mutex> lock(mClientMutex);
    return mClientSocket != kInvalidSocket;
}

ChannelState TcpChannel::currentState() const
{
    return mChannelState.load(std::memory_order_acquire);
}

bool TcpChannel::isListening() const
{
    return currentState() == ChannelState::Listening;
}

int TcpChannel::listeningPort() const
{
    return mPort.load();
}

// ════════════════════════════════════════════════
//  I/O Thread
// ════════════════════════════════════════════════

void TcpChannel::ioLoop()
{
    TC_LOG_DEBUG("I/O thread started");

    while (true)
    {
        auto st = currentState();
        if (st == ChannelState::Stopping || st == ChannelState::Terminated)
        {
            break;
        }

        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(mServerSocket, &readFds);

#ifndef _WIN32
        SocketHandle maxFd = mServerSocket;
#endif

        SocketHandle clientSock;
        {
            std::lock_guard<std::mutex> lock(mClientMutex);
            clientSock = mClientSocket;
        }

        if (clientSock != kInvalidSocket)
        {
            FD_SET(clientSock, &readFds);
#ifndef _WIN32
            if (clientSock > maxFd)
            {
                maxFd = clientSock;
            }
#endif
        }

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = kSelectTimeoutMs * 1000;

#ifdef _WIN32
        int nfds = 0;  // ignored on Windows
#else
        int nfds = static_cast<int>(maxFd) + 1;
#endif

        int ready = ::select(nfds, &readFds, nullptr, nullptr, &tv);
        if (ready < 0)
        {
            if (auto st = currentState(); st != ChannelState::Stopping && st != ChannelState::Terminated)
            {
                TC_LOG_ERROR("select() failed: " << detail::SocketHelper::lastError());
            }
            break;
        }
        if (ready == 0)
        {
            continue;  // timeout — loop back to check mChannelState
        }

        // New connection on server socket?
        if (FD_ISSET(mServerSocket, &readFds))
        {
            handleNewConnection();
        }

        // Data from client?
        // Re-read clientSock since handleNewConnection might have changed it
        {
            std::lock_guard<std::mutex> lock(mClientMutex);
            clientSock = mClientSocket;
        }
        if (clientSock != kInvalidSocket && FD_ISSET(clientSock, &readFds))
        {
            handleClientData();
        }
    }

    TC_LOG_DEBUG("I/O thread exiting");

    // Deferred cleanup: if stop() was called from a callback on this thread,
    // it set Stopping but could not join/cleanup. We do it here.
    // Also handles select() error exit when state is still Listening/Starting.
    if (beginStop())
    {
        TC_LOG_INFO("ioLoop detected unclean exit, transitioning to Stopping");
    }
    if (currentState() == ChannelState::Stopping)
    {
        TC_LOG_INFO("Deferred cleanup in ioLoop exit");
        finalizeStop();
    }
}

void TcpChannel::handleNewConnection()
{
    SocketHandle newSock = detail::SocketHelper::acceptClient(mServerSocket);
    if (newSock == kInvalidSocket)
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mClientMutex);
        if (mClientSocket != kInvalidSocket)
        {
            TC_LOG_WARN("Rejecting new connection — already have a client");
            detail::SocketHelper::closeSocket(newSock);
            return;
        }
        detail::SocketHelper::setNonBlocking(newSock);
        mClientSocket = newSock;
    }

    TC_LOG_INFO("Client connected");
    fireNotification(&ITcpChannelCallback::onClientConnected);
}

void TcpChannel::handleClientData()
{
    char buffer[kRecvBufferSize];

    SocketHandle sock;
    {
        std::lock_guard<std::mutex> lock(mClientMutex);
        sock = mClientSocket;
    }
    if (sock == kInvalidSocket)
    {
        return;
    }

    int n = detail::SocketHelper::recvNonBlocking(sock, buffer, kRecvBufferSize);
    if (n > 0)
    {
        std::string data(buffer, static_cast<size_t>(n));
        fireNotification(&ITcpChannelCallback::onDataReceived, data);
    }
    else if (n < 0)
    {
        // peer closed or error
        TC_LOG_INFO("Client disconnected (recv returned " << n << ")");
        disconnectClient();
        fireNotification(&ITcpChannelCallback::onClientDisconnected);
    }
    // n == 0: WOULDBLOCK after select — unlikely, just ignore
}

void TcpChannel::disconnectClient()
{
    std::lock_guard<std::mutex> lock(mClientMutex);
    detail::SocketHelper::closeSocket(mClientSocket);
}

void TcpChannel::finalizeStop()
{
    {
        std::lock_guard<std::mutex> lock(mClientMutex);
        detail::SocketHelper::closeSocket(mClientSocket);
    }
    detail::SocketHelper::closeSocket(mServerSocket);
    mPort = 0;
    tryTransition(ChannelState::Terminated);
}

} // namespace ucf::utilities
