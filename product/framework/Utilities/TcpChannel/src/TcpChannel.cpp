#include "TcpChannel.h"
#include "TcpChannelLogger.h"

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
//  Lifecycle
// ════════════════════════════════════════════════

TcpChannel::TcpChannel()
{
    detail::SocketHelper::initialize();
}

TcpChannel::~TcpChannel()
{
    stop();
    detail::SocketHelper::cleanup();
}

bool TcpChannel::startListening(const TcpChannelConfig& config)
{
    // CAS Idle → Starting: only one caller can win
    auto expected = ChannelState::Idle;
    if (!mChannelState.compare_exchange_strong(expected, ChannelState::Starting))
    {
        TC_LOG_WARN("startListening() rejected, state=" << static_cast<int>(expected));
        return false;
    }

    // Clean up a previous I/O thread that may still be joinable
    // (e.g. after a start → stop → start cycle)
    if (mIoThread.joinable())
    {
        mIoThread.join();
    }

    mConfig = config;
    mStopRequested = false;

    // Create server socket
    mServerSocket = detail::SocketHelper::createTcp();
    if (mServerSocket == kInvalidSocket)
    {
        std::string err = "Failed to create socket: " + detail::SocketHelper::lastError();
        TC_LOG_ERROR(err);
        mChannelState = ChannelState::Idle;
        fireNotification(&ITcpChannelCallback::onError, err);
        return false;
    }

    detail::SocketHelper::setReuseAddr(mServerSocket);

    if (!detail::SocketHelper::bindAndListen(mServerSocket, config.listenAddress,
                                              config.listenPort, config.maxConnections))
    {
        std::string err = "Failed to bind/listen on port " + std::to_string(config.listenPort)
                          + ": " + detail::SocketHelper::lastError();
        TC_LOG_ERROR(err);
        detail::SocketHelper::closeSocket(mServerSocket);
        mChannelState = ChannelState::Idle;
        fireNotification(&ITcpChannelCallback::onError, err);
        return false;
    }

    mPort = detail::SocketHelper::getLocalPort(mServerSocket);

    TC_LOG_INFO("Listening on " << config.listenAddress << ":" << mPort.load());

    // Start I/O thread BEFORE setting Listening —
    // ensures the thread object is valid when stop() tries to join
    mIoThread = std::thread(&TcpChannel::ioLoop, this);
    mChannelState = ChannelState::Listening;
    return true;
}

void TcpChannel::stop()
{
    // CAS Listening → Stopping: only one caller can win
    auto expected = ChannelState::Listening;
    if (!mChannelState.compare_exchange_strong(expected, ChannelState::Stopping))
    {
        TC_LOG_DEBUG("stop() skipped, state=" << static_cast<int>(expected));
        return;
    }

    TC_LOG_INFO("Stopping TcpChannel");
    mStopRequested = true;

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

    cleanupSockets();
    mChannelState = ChannelState::Idle;
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

bool TcpChannel::isListening() const
{
    return mChannelState.load() == ChannelState::Listening;
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

    while (!mStopRequested.load())
    {
        fd_set readFds;
        FD_ZERO(&readFds);

        SocketHandle maxFd = mServerSocket;
        FD_SET(mServerSocket, &readFds);

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
            if (!mStopRequested.load())
            {
                TC_LOG_ERROR("select() failed: " << detail::SocketHelper::lastError());
            }
            break;
        }
        if (ready == 0)
        {
            continue;  // timeout — loop back to check mStopRequested
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
    auto expected = ChannelState::Stopping;
    if (mChannelState.compare_exchange_strong(expected, ChannelState::Idle))
    {
        TC_LOG_INFO("Deferred cleanup in ioLoop exit");
        cleanupSockets();
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

void TcpChannel::cleanupSockets()
{
    {
        std::lock_guard<std::mutex> lock(mClientMutex);
        detail::SocketHelper::closeSocket(mClientSocket);
    }
    detail::SocketHelper::closeSocket(mServerSocket);
    mPort = 0;
}

} // namespace ucf::utilities
