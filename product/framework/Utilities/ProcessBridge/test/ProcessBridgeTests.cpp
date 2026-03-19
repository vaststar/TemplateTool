#include <catch2/catch_test_macros.hpp>

#include <ucf/Utilities/ProcessBridge/IProcessBridge.h>
#include <ucf/Utilities/ProcessBridge/IProcessBridgeCallback.h>
#include <ucf/Utilities/ProcessBridge/ProcessBridgeConfig.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

using namespace ucf::utilities;

// ════════════════════════════════════════════════════════════
//  Test callback that records events
// ════════════════════════════════════════════════════════════

class TestProcessCallback : public IProcessBridgeCallback
{
public:
    void onProcessStarted(int64_t pid) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        startedPid = pid;
        started = true;
        mCv.notify_all();
    }

    void onProcessStopped(int exitCode, bool crashed) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        stoppedExitCode = exitCode;
        stoppedCrashed = crashed;
        stopped = true;
        mCv.notify_all();
    }

    void onProcessError(const std::string& errorMessage) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        errorMsg = errorMessage;
        errored = true;
        mCv.notify_all();
    }

    void onStdout(const std::string& data) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        stdoutData += data;
        mCv.notify_all();
    }

    void onStderr(const std::string& data) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        stderrData += data;
        mCv.notify_all();
    }

    bool waitForStarted(int timeoutMs = 5000)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return mCv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                            [this] { return started.load(); });
    }

    bool waitForStopped(int timeoutMs = 5000)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return mCv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                            [this] { return stopped.load(); });
    }

    bool waitForError(int timeoutMs = 5000)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return mCv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                            [this] { return errored.load(); });
    }

    std::atomic<bool> started{false};
    std::atomic<bool> stopped{false};
    std::atomic<bool> errored{false};
    int64_t startedPid{0};
    int stoppedExitCode{-1};
    bool stoppedCrashed{false};
    std::string errorMsg;
    std::string stdoutData;
    std::string stderrData;

private:
    std::mutex mMutex;
    std::condition_variable mCv;
};

// ════════════════════════════════════════════════════════════
//  Tests
// ════════════════════════════════════════════════════════════

TEST_CASE("ProcessBridge creation", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    REQUIRE(bridge != nullptr);
    REQUIRE(bridge->state() == ProcessState::Idle);
    REQUIRE_FALSE(bridge->isRunning());
    REQUIRE(bridge->processPid() == 0);
}

TEST_CASE("ProcessBridge error on invalid executable", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
    config.executablePath = "/non/existent/path/no_such_executable_12345";

    bool result = bridge->start(config);
#ifdef _WIN32
    // Windows: CreateProcess fails immediately
    REQUIRE_FALSE(result);
    REQUIRE(bridge->state() == ProcessState::Terminated);
#else
    // Unix: fork() succeeds but execvp fails — child exits with code 127
    REQUIRE(result);
    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stoppedExitCode == 127);
#endif
}

#ifdef _WIN32

TEST_CASE("ProcessBridge launches and stops cmd.exe", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "echo hello"};

    bool result = bridge->start(config);
    REQUIRE(result);
    REQUIRE(bridge->isRunning());
    REQUIRE(bridge->processPid() > 0);

    // Wait for the process to finish on its own (echo is very fast)
    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->started);
    REQUIRE(callback->stopped);
    REQUIRE(callback->stdoutData.find("hello") != std::string::npos);
}

TEST_CASE("ProcessBridge captures stderr on cmd.exe", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "echo errmsg 1>&2"};

    bool result = bridge->start(config);
    REQUIRE(result);

    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stderrData.find("errmsg") != std::string::npos);
}

TEST_CASE("ProcessBridge stop kills long-running process", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "ping -n 30 127.0.0.1 > nul"};
    config.stopTimeoutMs = 500;

    bool result = bridge->start(config);
    REQUIRE(result);
    REQUIRE(callback->waitForStarted(3000));
    REQUIRE(bridge->isRunning());

    bridge->stop();

    REQUIRE(bridge->state() == ProcessState::Terminated);
    REQUIRE_FALSE(bridge->isRunning());
}

#else  // Unix

TEST_CASE("ProcessBridge launches and captures stdout on Unix", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
    config.executablePath = "/bin/echo";
    config.arguments = {"hello"};

    bool result = bridge->start(config);
    REQUIRE(result);

    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stdoutData.find("hello") != std::string::npos);
}

TEST_CASE("ProcessBridge captures stderr on Unix", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
    config.executablePath = "/bin/sh";
    config.arguments = {"-c", "echo errmsg >&2"};

    bool result = bridge->start(config);
    REQUIRE(result);

    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stderrData.find("errmsg") != std::string::npos);
}

TEST_CASE("ProcessBridge stop kills long-running process on Unix", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
    config.executablePath = "/bin/sleep";
    config.arguments = {"30"};
    config.stopTimeoutMs = 500;

    bool result = bridge->start(config);
    REQUIRE(result);
    REQUIRE(callback->waitForStarted(3000));

    bridge->stop();

    REQUIRE(bridge->state() == ProcessState::Terminated);
    REQUIRE_FALSE(bridge->isRunning());
}

TEST_CASE("ProcessBridge passes arguments with spaces on Unix", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
    config.executablePath = "/bin/echo";
    config.arguments = {"hello world", "foo bar"};

    bool result = bridge->start(config);
    REQUIRE(result);

    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stdoutData.find("hello world") != std::string::npos);
    REQUIRE(callback->stdoutData.find("foo bar") != std::string::npos);
}

#endif

TEST_CASE("ProcessBridge double start returns false", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();

    ProcessBridgeConfig config;
#ifdef _WIN32
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "ping -n 10 127.0.0.1 > nul"};
#else
    config.executablePath = "/bin/sleep";
    config.arguments = {"10"};
#endif

    bool first = bridge->start(config);
    REQUIRE(first);

    bool second = bridge->start(config);
    REQUIRE_FALSE(second);

    bridge->stop();
}

TEST_CASE("ProcessBridge callback weak_ptr safety", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();

    {
        auto callback = std::make_shared<TestProcessCallback>();
        bridge->registerCallback(callback);
        // callback goes out of scope — weak_ptr should be properly cleaned up
    }

    // Starting after callback is destroyed should not crash
    ProcessBridgeConfig config;
#ifdef _WIN32
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "echo safe"};
#else
    config.executablePath = "/bin/echo";
    config.arguments = {"safe"};
#endif

    bool result = bridge->start(config);
    REQUIRE(result);

    // Give the process time to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    bridge->stop();
    // No crash = pass
}

TEST_CASE("ProcessBridge restart after Terminated", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    // First run
    ProcessBridgeConfig config;
#ifdef _WIN32
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "echo first"};
#else
    config.executablePath = "/bin/echo";
    config.arguments = {"first"};
#endif

    REQUIRE(bridge->start(config));
    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(bridge->state() == ProcessState::Terminated);

    // Reset callback state
    callback->started = false;
    callback->stopped = false;
    callback->stdoutData.clear();

    // Second run — should succeed from Terminated state
#ifdef _WIN32
    config.arguments = {"/C", "echo second"};
#else
    config.arguments = {"second"};
#endif

    REQUIRE(bridge->start(config));
    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stdoutData.find("second") != std::string::npos);
}

TEST_CASE("ProcessBridge natural exit with non-zero code", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
#ifdef _WIN32
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "exit 42"};
#else
    config.executablePath = "/bin/sh";
    config.arguments = {"-c", "exit 42"};
#endif

    REQUIRE(bridge->start(config));
    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stoppedExitCode == 42);
    REQUIRE(callback->stoppedCrashed == true);
    REQUIRE(bridge->state() == ProcessState::Terminated);
    REQUIRE_FALSE(bridge->isRunning());
    REQUIRE(bridge->processPid() == 0);
}

TEST_CASE("ProcessBridge stop on idle is no-op", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    REQUIRE(bridge->state() == ProcessState::Idle);
    bridge->stop();  // should not crash or change state
    REQUIRE(bridge->state() == ProcessState::Idle);
}

// ════════════════════════════════════════════════════════════
//  State machine & thread-safety tests
// ════════════════════════════════════════════════════════════

TEST_CASE("ProcessBridge stop on Running reports clean termination", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
#ifdef _WIN32
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "ping -n 30 127.0.0.1 > nul"};
#else
    config.executablePath = "/bin/sleep";
    config.arguments = {"30"};
#endif
    config.stopTimeoutMs = 500;

    REQUIRE(bridge->start(config));
    REQUIRE(callback->waitForStarted(3000));

    bridge->stop();

    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stoppedCrashed == false);
    REQUIRE(bridge->state() == ProcessState::Terminated);
}

TEST_CASE("ProcessBridge stop on Terminated is no-op", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
#ifdef _WIN32
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "echo hi"};
#else
    config.executablePath = "/bin/echo";
    config.arguments = {"hi"};
#endif

    REQUIRE(bridge->start(config));
    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(bridge->state() == ProcessState::Terminated);

    // stop() on Terminated should not crash or change state
    bridge->stop();
    REQUIRE(bridge->state() == ProcessState::Terminated);
}

TEST_CASE("ProcessBridge captureStdout=false suppresses stdout callback", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
#ifdef _WIN32
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "echo suppressed"};
#else
    config.executablePath = "/bin/echo";
    config.arguments = {"suppressed"};
#endif
    config.captureStdout = false;

    REQUIRE(bridge->start(config));
    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stdoutData.empty());
}

TEST_CASE("ProcessBridge captureStderr=false suppresses stderr callback", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
#ifdef _WIN32
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "echo suppressed 1>&2"};
#else
    config.executablePath = "/bin/sh";
    config.arguments = {"-c", "echo suppressed >&2"};
#endif
    config.captureStderr = false;

    REQUIRE(bridge->start(config));
    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stderrData.empty());
}

TEST_CASE("ProcessBridge stop from onStdout callback (deferred cleanup)", "[ProcessBridge]")
{
    // Simulate stop() being called from inside a callback (on the monitor thread).
    // This exercises the self-join detection + deferred terminate/cleanup path.

    struct StopOnOutputCallback : public IProcessBridgeCallback
    {
        IProcessBridge* bridge{nullptr};
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic<bool> stopped{false};
        int exitCode{-1};

        void onProcessStarted(int64_t /*pid*/) override {}
        void onProcessStopped(int ec, bool /*crashed*/) override
        {
            std::lock_guard<std::mutex> lock(mtx);
            exitCode = ec;
            stopped = true;
            cv.notify_all();
        }
        void onProcessError(const std::string& /*msg*/) override {}
        void onStdout(const std::string& /*data*/) override
        {
            // Called on monitor thread — triggers deferred stop path
            if (bridge)
            {
                bridge->stop();
            }
        }
        void onStderr(const std::string& /*data*/) override {}

        bool waitForStopped(int timeoutMs = 10000)
        {
            std::unique_lock<std::mutex> lock(mtx);
            return cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                               [this] { return stopped.load(); });
        }
    };

    auto bridge = IProcessBridge::create();
    auto cb = std::make_shared<StopOnOutputCallback>();
    cb->bridge = bridge.get();
    bridge->registerCallback(cb);

    ProcessBridgeConfig config;
#ifdef _WIN32
    // Use a command that outputs something and keeps running
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "echo trigger && ping -n 10 127.0.0.1 > nul"};
#else
    config.executablePath = "/bin/sh";
    config.arguments = {"-c", "echo trigger; sleep 10"};
#endif
    config.stopTimeoutMs = 2000;

    REQUIRE(bridge->start(config));
    REQUIRE(cb->waitForStopped(15000));
    REQUIRE(bridge->state() == ProcessState::Terminated);
}

TEST_CASE("ProcessBridge concurrent stop from two threads", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
#ifdef _WIN32
    config.executablePath = "cmd.exe";
    config.arguments = {"/C", "ping -n 30 127.0.0.1 > nul"};
#else
    config.executablePath = "/bin/sleep";
    config.arguments = {"30"};
#endif
    config.stopTimeoutMs = 500;

    REQUIRE(bridge->start(config));
    REQUIRE(callback->waitForStarted(3000));

    // Two threads race to stop — only one should succeed, no crash
    std::thread t1([&] { bridge->stop(); });
    std::thread t2([&] { bridge->stop(); });
    t1.join();
    t2.join();

    REQUIRE(bridge->state() == ProcessState::Terminated);
}

TEST_CASE("ProcessBridge multiple restart cycles", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    for (int i = 0; i < 3; ++i)
    {
        callback->started = false;
        callback->stopped = false;
        callback->stdoutData.clear();

        ProcessBridgeConfig config;
#ifdef _WIN32
        config.executablePath = "cmd.exe";
        config.arguments = {"/C", "echo cycle" + std::to_string(i)};
#else
        config.executablePath = "/bin/echo";
        config.arguments = {"cycle" + std::to_string(i)};
#endif

        REQUIRE(bridge->start(config));
        REQUIRE(callback->waitForStopped(5000));
        REQUIRE(bridge->state() == ProcessState::Terminated);

        std::string expected = "cycle" + std::to_string(i);
        REQUIRE(callback->stdoutData.find(expected) != std::string::npos);
    }
}

TEST_CASE("ProcessBridge concurrent start and immediate stop", "[ProcessBridge]")
{
    // Exercises the race condition: stop() can intervene while start() is
    // still in progress (Starting state). The state machine + lifecycle mutex
    // must ensure no data race on mHandle/mMonitorThread.
    for (int i = 0; i < 10; ++i)
    {
        auto bridge = IProcessBridge::create();
        auto callback = std::make_shared<TestProcessCallback>();
        bridge->registerCallback(callback);

        ProcessBridgeConfig config;
#ifdef _WIN32
        config.executablePath = "cmd.exe";
        config.arguments = {"/C", "ping -n 10 127.0.0.1 > nul"};
#else
        config.executablePath = "/bin/sleep";
        config.arguments = {"10"};
#endif
        config.stopTimeoutMs = 500;

        // Fire start and immediately stop from another thread
        std::thread stopThread;
        bool startResult = bridge->start(config);

        if (startResult)
        {
            stopThread = std::thread([&] { bridge->stop(); });
        }

        if (stopThread.joinable())
        {
            stopThread.join();
        }

        // Must end in a terminal state, no crash, no hanging thread
        auto finalState = bridge->state();
        REQUIRE((finalState == ProcessState::Terminated
                 || finalState == ProcessState::Idle));
    }
}
