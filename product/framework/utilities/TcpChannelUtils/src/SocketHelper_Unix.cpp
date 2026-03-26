#if !defined(_WIN32)

#include "SocketHelper.h"

namespace ucf::utilities::detail {

bool SocketHelper::initialize()
{
    return true;
}

void SocketHelper::cleanup()
{
}

SocketHandle SocketHelper::createTcp()
{
    return ::socket(AF_INET, SOCK_STREAM, 0);
}

bool SocketHelper::setReuseAddr(SocketHandle sock)
{
    int opt = 1;
    return ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
}

bool SocketHelper::bindAndListen(SocketHandle sock, const std::string& address, int port, int backlog)
{
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) != 1)
    {
        return false;
    }

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        return false;
    }
    return ::listen(sock, backlog) == 0;
}

SocketHandle SocketHelper::acceptClient(SocketHandle serverSock)
{
    return ::accept(serverSock, nullptr, nullptr);
}

int SocketHelper::getLocalPort(SocketHandle sock)
{
    sockaddr_in addr{};
    socklen_t addrLen = sizeof(addr);
    if (::getsockname(sock, reinterpret_cast<sockaddr*>(&addr), &addrLen) == 0)
    {
        return ntohs(addr.sin_port);
    }
    return -1;
}

bool SocketHelper::sendAll(SocketHandle sock, const char* data, int len)
{
    int sent = 0;
    while (sent < len)
    {
        ssize_t n = ::send(sock, data + sent, len - sent, MSG_NOSIGNAL);
        if (n <= 0)
        {
            return false;
        }
        sent += static_cast<int>(n);
    }
    return true;
}

int SocketHelper::recvNonBlocking(SocketHandle sock, char* buffer, int bufferLen)
{
    ssize_t n = ::recv(sock, buffer, bufferLen, 0);
    if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return 0;  // no data available
        }
        return -1;  // real error
    }
    if (n == 0)
    {
        return -1;  // peer closed
    }
    return static_cast<int>(n);
}

bool SocketHelper::setNonBlocking(SocketHandle sock)
{
    int flags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) == 0;
}

void SocketHelper::closeSocket(SocketHandle& sock)
{
    if (sock != kInvalidSocket)
    {
        ::close(sock);
        sock = kInvalidSocket;
    }
}

std::string SocketHelper::lastError()
{
    return std::string(strerror(errno));
}

} // namespace ucf::utilities::detail

#endif // !_WIN32
