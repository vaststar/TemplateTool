#ifdef _WIN32

#include "SocketHelper.h"

#include <atomic>

namespace ucf::utilities::detail {

static std::atomic<int> sInitCount{0};

bool SocketHelper::initialize()
{
    if (sInitCount.fetch_add(1) == 0)
    {
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    }
    return true;
}

void SocketHelper::cleanup()
{
    if (sInitCount.fetch_sub(1) == 1)
    {
        WSACleanup();
    }
}

SocketHandle SocketHelper::createTcp()
{
    return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

bool SocketHelper::setReuseAddr(SocketHandle sock)
{
    int opt = 1;
    return ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                        reinterpret_cast<const char*>(&opt), sizeof(opt)) == 0;
}

bool SocketHelper::bindAndListen(SocketHandle sock, const std::string& address, int port, int backlog)
{
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<u_short>(port));
    inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

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
    int addrLen = sizeof(addr);
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
        int n = ::send(sock, data + sent, len - sent, 0);
        if (n <= 0)
        {
            return false;
        }
        sent += n;
    }
    return true;
}

int SocketHelper::recvNonBlocking(SocketHandle sock, char* buffer, int bufferLen)
{
    int n = ::recv(sock, buffer, bufferLen, 0);
    if (n < 0)
    {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
        {
            return 0;  // no data available
        }
        return -1;  // real error
    }
    if (n == 0)
    {
        return -1;  // peer closed
    }
    return n;
}

bool SocketHelper::setNonBlocking(SocketHandle sock)
{
    u_long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode) == 0;
}

void SocketHelper::closeSocket(SocketHandle& sock)
{
    if (sock != kInvalidSocket)
    {
        ::closesocket(sock);
        sock = kInvalidSocket;
    }
}

std::string SocketHelper::lastError()
{
    int err = WSAGetLastError();
    char buf[256] = {};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, static_cast<DWORD>(err),
                   0, buf, sizeof(buf), nullptr);
    return std::string(buf);
}

} // namespace ucf::utilities::detail

#endif // _WIN32
