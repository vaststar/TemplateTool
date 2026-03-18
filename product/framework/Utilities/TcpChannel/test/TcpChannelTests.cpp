#include <catch2/catch_test_macros.hpp>

#include <ucf/Utilities/TcpChannel/ITcpChannel.h>
#include <ucf/Utilities/TcpChannel/ITcpChannelCallback.h>
#include <ucf/Utilities/TcpChannel/TcpChannelConfig.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")

    static void initWsa()
    {
        static bool done = false;
        if (!done)
        {
            WSADATA wsa;
            WSAStartup(MAKEWORD(2, 2), &wsa);
            done = true;
        }
    }
    using SocketType = SOCKET;
    static constexpr SocketType kBadSock = INVALID_SOCKET;
    static void closeSock(SocketType s)
    {
        ::closesocket(s);
    }
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>

    static void initWsa() {}
    using SocketType = int;
    static constexpr SocketType kBadSock = -1;
    static void closeSock(SocketType s)
    {
        ::close(s);
    }
#endif

using namespace ucf::utilities;

// ════════════════════════════════════════════════════════════
//  Helper: connect a raw client socket to localhost:port
// ════════════════════════════════════════════════════════════

static SocketType connectToLocal(int port)
{
    initWsa();
    SocketType s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == kBadSock)
    {
        return kBadSock;
    }

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (::connect(s, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        closeSock(s);
        return kBadSock;
    }
    return s;
}

// ════════════════════════════════════════════════════════════
//  Test callback that records events
// ════════════════════════════════════════════════════════════

class TestTcpCallback : public ITcpChannelCallback
{
public:
    void onClientConnected() override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        connected = true;
        mCv.notify_all();
    }

    void onClientDisconnected() override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        disconnected = true;
        mCv.notify_all();
    }

    void onDataReceived(const std::string& data) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        receivedData += data;
        mCv.notify_all();
    }

    void onError(const std::string& errorMessage) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        errorMsg = errorMessage;
        errored = true;
        mCv.notify_all();
    }

    bool waitForConnected(int timeoutMs = 3000)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return mCv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                            [this] { return connected.load(); });
    }

    bool waitForDisconnected(int timeoutMs = 3000)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return mCv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                            [this] { return disconnected.load(); });
    }

    bool waitForData(int timeoutMs = 3000)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return mCv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                            [this] { return !receivedData.empty(); });
    }

    std::atomic<bool> connected{false};
    std::atomic<bool> disconnected{false};
    std::atomic<bool> errored{false};
    std::string receivedData;
    std::string errorMsg;

private:
    std::mutex mMutex;
    std::condition_variable mCv;
};

// ════════════════════════════════════════════════════════════
//  Tests
// ════════════════════════════════════════════════════════════

TEST_CASE("TcpChannel creation", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();
    REQUIRE(channel != nullptr);
    REQUIRE_FALSE(channel->isListening());
    REQUIRE_FALSE(channel->isConnected());
    REQUIRE(channel->listeningPort() == 0);
}

TEST_CASE("TcpChannel listen on ephemeral port", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;  // let OS pick
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));
    REQUIRE(channel->isListening());
    REQUIRE(channel->listeningPort() > 0);

    channel->stop();
    REQUIRE_FALSE(channel->isListening());
}

TEST_CASE("TcpChannel double start returns false", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));
    REQUIRE_FALSE(channel->startListening(config));  // second start fails

    channel->stop();
}

TEST_CASE("TcpChannel client connect and send data", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();
    auto cb = std::make_shared<TestTcpCallback>();
    channel->registerCallback(cb);

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));
    int port = channel->listeningPort();

    // Connect a raw client
    SocketType client = connectToLocal(port);
    REQUIRE(client != kBadSock);
    REQUIRE(cb->waitForConnected());
    REQUIRE(channel->isConnected());

    // Client sends data to server
    const char* msg = "hello from client";
    ::send(client, msg, static_cast<int>(std::strlen(msg)), 0);
    REQUIRE(cb->waitForData());
    REQUIRE(cb->receivedData == "hello from client");

    closeSock(client);
    REQUIRE(cb->waitForDisconnected());
    REQUIRE_FALSE(channel->isConnected());

    channel->stop();
}

TEST_CASE("TcpChannel server sends data to client", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();
    auto cb = std::make_shared<TestTcpCallback>();
    channel->registerCallback(cb);

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));
    int port = channel->listeningPort();

    SocketType client = connectToLocal(port);
    REQUIRE(client != kBadSock);
    REQUIRE(cb->waitForConnected());

    // Server sends to client
    REQUIRE(channel->send("hello from server"));

    // Client reads
    char buf[256] = {};
    int n = ::recv(client, buf, sizeof(buf) - 1, 0);
    REQUIRE(n > 0);
    REQUIRE(std::string(buf, n) == "hello from server");

    closeSock(client);
    channel->stop();
}

TEST_CASE("TcpChannel stop closes connection", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();
    auto cb = std::make_shared<TestTcpCallback>();
    channel->registerCallback(cb);

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));
    int port = channel->listeningPort();

    SocketType client = connectToLocal(port);
    REQUIRE(client != kBadSock);
    REQUIRE(cb->waitForConnected());

    channel->stop();
    REQUIRE_FALSE(channel->isListening());
    REQUIRE_FALSE(channel->isConnected());

    // Client should detect closure
    char buf[64];
    int n = ::recv(client, buf, sizeof(buf), 0);
    REQUIRE(n <= 0);

    closeSock(client);
}

TEST_CASE("TcpChannel send without client returns false", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));
    REQUIRE_FALSE(channel->send("should fail"));

    channel->stop();
}
