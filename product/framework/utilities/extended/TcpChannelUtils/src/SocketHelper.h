#pragma once

#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using SocketHandle = SOCKET;
    constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <cerrno>
    #include <cstring>
    using SocketHandle = int;
    constexpr SocketHandle kInvalidSocket = -1;
#endif

namespace ucf::utilities::detail {

/// Low-level cross-platform socket operations.
/// This is an internal helper — not part of the public API.
class SocketHelper
{
public:
    /// Platform initialization (Winsock on Windows, no-op on Unix).
    static bool initialize();
    static void cleanup();

    /// Create a TCP socket.
    static SocketHandle createTcp();

    /// Enable SO_REUSEADDR on a socket.
    static bool setReuseAddr(SocketHandle sock);

    /// Bind to address:port and start listening.
    static bool bindAndListen(SocketHandle sock, const std::string& address, int port, int backlog);

    /// Accept a pending client connection (non-blocking after select).
    static SocketHandle acceptClient(SocketHandle serverSock);

    /// Get the actual local port a socket is bound to.
    static int getLocalPort(SocketHandle sock);

    /// Send all bytes (blocks until complete or error).
    static bool sendAll(SocketHandle sock, const char* data, int len);

    /// Non-blocking receive.
    /// @return >0 bytes read, 0 = no data (WOULDBLOCK), -1 = error or peer closed
    ///         Note: recv returning 0 from OS means peer closed,
    ///         mapped to -1 here. WOULDBLOCK mapped to 0.
    static int recvNonBlocking(SocketHandle sock, char* buffer, int bufferLen);

    /// Set socket to non-blocking mode.
    static bool setNonBlocking(SocketHandle sock);

    /// Close a socket and set handle to invalid.
    static void closeSocket(SocketHandle& sock);

    /// Human-readable description of the last socket error.
    static std::string lastError();
};

} // namespace ucf::utilities::detail
