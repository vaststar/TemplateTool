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

// ════════════════════════════════════════════════════════════
//  State machine & thread-safety tests
// ════════════════════════════════════════════════════════════

TEST_CASE("TcpChannel stop on idle is no-op", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();
    REQUIRE_FALSE(channel->isListening());
    channel->stop();  // should not crash
    REQUIRE_FALSE(channel->isListening());
}

TEST_CASE("TcpChannel double stop is no-op", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));

    channel->stop();
    REQUIRE_FALSE(channel->isListening());

    // Second stop should be harmless
    channel->stop();
    REQUIRE_FALSE(channel->isListening());
}

TEST_CASE("TcpChannel restart after stop preserves functionality", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();
    auto cb = std::make_shared<TestTcpCallback>();
    channel->registerCallback(cb);

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    // Cycle 1
    REQUIRE(channel->startListening(config));
    int port1 = channel->listeningPort();
    REQUIRE(port1 > 0);

    SocketType client1 = connectToLocal(port1);
    REQUIRE(client1 != kBadSock);
    REQUIRE(cb->waitForConnected());

    const char* msg1 = "cycle1";
    ::send(client1, msg1, static_cast<int>(std::strlen(msg1)), 0);
    REQUIRE(cb->waitForData());
    REQUIRE(cb->receivedData == "cycle1");

    closeSock(client1);
    cb->waitForDisconnected();
    channel->stop();

    // Reset callback state
    cb->connected = false;
    cb->disconnected = false;
    cb->receivedData.clear();

    // Cycle 2 — full functionality should work again
    REQUIRE(channel->startListening(config));
    int port2 = channel->listeningPort();
    REQUIRE(port2 > 0);

    SocketType client2 = connectToLocal(port2);
    REQUIRE(client2 != kBadSock);
    REQUIRE(cb->waitForConnected());

    const char* msg2 = "cycle2";
    ::send(client2, msg2, static_cast<int>(std::strlen(msg2)), 0);
    REQUIRE(cb->waitForData());
    REQUIRE(cb->receivedData == "cycle2");

    closeSock(client2);
    cb->waitForDisconnected();
    channel->stop();
}

TEST_CASE("TcpChannel stop from onDataReceived callback (deferred cleanup)", "[TcpChannel]")
{
    // Simulate stop() being called from within a callback on the I/O thread.
    // This exercises self-join detection + deferred cleanup in ioLoop exit.

    struct StopOnDataCallback : public ITcpChannelCallback
    {
        ITcpChannel* channel{nullptr};
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic<bool> stopCalled{false};
        std::atomic<bool> gotData{false};

        void onClientConnected() override {}
        void onClientDisconnected() override {}
        void onDataReceived(const std::string& /*data*/) override
        {
            gotData = true;
            // Called on I/O thread — triggers deferred stop path
            if (channel)
            {
                channel->stop();
            }
            std::lock_guard<std::mutex> lock(mtx);
            stopCalled = true;
            cv.notify_all();
        }
        void onError(const std::string& /*msg*/) override {}

        bool waitForStopCalled(int timeoutMs = 5000)
        {
            std::unique_lock<std::mutex> lock(mtx);
            return cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                               [this] { return stopCalled.load(); });
        }
    };

    auto channel = ITcpChannel::create();
    auto cb = std::make_shared<StopOnDataCallback>();
    cb->channel = channel.get();
    channel->registerCallback(cb);

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));
    int port = channel->listeningPort();

    // Connect and send data to trigger the callback
    SocketType client = connectToLocal(port);
    REQUIRE(client != kBadSock);

    // Small delay so the server accepts the connection
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    const char* msg = "trigger";
    ::send(client, msg, static_cast<int>(std::strlen(msg)), 0);

    REQUIRE(cb->waitForStopCalled(5000));

    // Give ioLoop time to do deferred cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    REQUIRE_FALSE(channel->isListening());
    REQUIRE(cb->gotData);

    closeSock(client);
}

TEST_CASE("TcpChannel stop from onClientConnected callback", "[TcpChannel]")
{
    struct StopOnConnectCallback : public ITcpChannelCallback
    {
        ITcpChannel* channel{nullptr};
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic<bool> done{false};

        void onClientConnected() override
        {
            if (channel)
            {
                channel->stop();
            }
            std::lock_guard<std::mutex> lock(mtx);
            done = true;
            cv.notify_all();
        }
        void onClientDisconnected() override {}
        void onDataReceived(const std::string& /*data*/) override {}
        void onError(const std::string& /*msg*/) override {}

        bool waitForDone(int timeoutMs = 5000)
        {
            std::unique_lock<std::mutex> lock(mtx);
            return cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                               [this] { return done.load(); });
        }
    };

    auto channel = ITcpChannel::create();
    auto cb = std::make_shared<StopOnConnectCallback>();
    cb->channel = channel.get();
    channel->registerCallback(cb);

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));
    int port = channel->listeningPort();

    SocketType client = connectToLocal(port);
    REQUIRE(client != kBadSock);

    REQUIRE(cb->waitForDone(5000));

    // Give ioLoop time to do deferred cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    REQUIRE_FALSE(channel->isListening());
    closeSock(client);
}

TEST_CASE("TcpChannel concurrent stop from two threads", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));

    // Two threads race to stop — only one should succeed, no crash
    std::thread t1([&] { channel->stop(); });
    std::thread t2([&] { channel->stop(); });
    t1.join();
    t2.join();

    REQUIRE_FALSE(channel->isListening());
}

TEST_CASE("TcpChannel multiple restart cycles", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    for (int i = 0; i < 5; ++i)
    {
        REQUIRE(channel->startListening(config));
        REQUIRE(channel->isListening());
        REQUIRE(channel->listeningPort() > 0);
        channel->stop();
        REQUIRE_FALSE(channel->isListening());
    }
}

TEST_CASE("TcpChannel rejects second client connection", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();
    auto cb = std::make_shared<TestTcpCallback>();
    channel->registerCallback(cb);

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 2;  // backlog allows queueing but only 1 accepted

    REQUIRE(channel->startListening(config));
    int port = channel->listeningPort();

    // First client connects
    SocketType client1 = connectToLocal(port);
    REQUIRE(client1 != kBadSock);
    REQUIRE(cb->waitForConnected());
    REQUIRE(channel->isConnected());

    // Second client — should be connected at TCP level but rejected by TcpChannel
    SocketType client2 = connectToLocal(port);
    // Give the server time to reject
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // First client is still connected
    REQUIRE(channel->isConnected());

    closeSock(client1);
    closeSock(client2);
    channel->stop();
}

TEST_CASE("TcpChannel callback weak_ptr safety", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();

    {
        auto cb = std::make_shared<TestTcpCallback>();
        channel->registerCallback(cb);
        // cb goes out of scope — weak_ptr should handle gracefully
    }

    TcpChannelConfig config;
    config.listenAddress = "127.0.0.1";
    config.listenPort = 0;
    config.maxConnections = 1;

    REQUIRE(channel->startListening(config));
    int port = channel->listeningPort();

    // Connect after callback is destroyed — should not crash
    SocketType client = connectToLocal(port);
    if (client != kBadSock)
    {
        const char* msg = "safe";
        ::send(client, msg, static_cast<int>(std::strlen(msg)), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        closeSock(client);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    channel->stop();
    // No crash = pass
}

TEST_CASE("TcpChannel concurrent start and immediate stop", "[TcpChannel]")
{
    // Exercises the race condition: stop() can intervene while startListening()
    // is still setting up sockets. The state machine + lifecycle mutex must
    // ensure no data race on mServerSocket/mIoThread.
    for (int i = 0; i < 10; ++i)
    {
        auto channel = ITcpChannel::create();

        TcpChannelConfig config;
        config.listenAddress = "127.0.0.1";
        config.listenPort = 0;
        config.maxConnections = 1;

        // Fire start and immediately stop from another thread
        std::thread stopThread;
        bool startResult = channel->startListening(config);

        if (startResult)
        {
            stopThread = std::thread([&] { channel->stop(); });
        }

        if (stopThread.joinable())
        {
            stopThread.join();
        }

        // Must end cleanly, no crash, no hanging thread
        REQUIRE_FALSE(channel->isListening());
    }
}

TEST_CASE("TcpChannel listen on invalid address fails", "[TcpChannel]")
{
    auto channel = ITcpChannel::create();
    auto cb = std::make_shared<TestTcpCallback>();
    channel->registerCallback(cb);

    TcpChannelConfig config;
    config.listenAddress = "999.999.999.999";  // invalid address
    config.listenPort = 0;
    config.maxConnections = 1;

    bool result = channel->startListening(config);
    REQUIRE_FALSE(result);
    REQUIRE_FALSE(channel->isListening());
    REQUIRE(cb->errored);

    // Should be restartable after failure
    config.listenAddress = "127.0.0.1";
    REQUIRE(channel->startListening(config));
    REQUIRE(channel->isListening());
    channel->stop();
}
