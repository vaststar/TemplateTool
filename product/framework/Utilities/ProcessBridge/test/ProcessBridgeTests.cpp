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
    config.captureStdout = false;
    config.captureStderr = false;

    bool result = bridge->start(config);
    REQUIRE_FALSE(result);
    REQUIRE(bridge->state() == ProcessState::Error);
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
    config.captureStdout = true;
    config.captureStderr = false;

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
    config.captureStdout = false;
    config.captureStderr = true;

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
    config.captureStdout = false;
    config.captureStderr = false;
    config.stopTimeoutMs = 500;

    bool result = bridge->start(config);
    REQUIRE(result);
    REQUIRE(callback->waitForStarted(3000));
    REQUIRE(bridge->isRunning());

    bridge->stop();

    REQUIRE(bridge->state() == ProcessState::Stopped);
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
    config.captureStdout = true;
    config.captureStderr = false;

    bool result = bridge->start(config);
    REQUIRE(result);

    REQUIRE(callback->waitForStopped(5000));
    REQUIRE(callback->stdoutData.find("hello") != std::string::npos);
}

TEST_CASE("ProcessBridge stop kills long-running process on Unix", "[ProcessBridge]")
{
    auto bridge = IProcessBridge::create();
    auto callback = std::make_shared<TestProcessCallback>();
    bridge->registerCallback(callback);

    ProcessBridgeConfig config;
    config.executablePath = "/bin/sleep";
    config.arguments = {"30"};
    config.captureStdout = false;
    config.captureStderr = false;
    config.stopTimeoutMs = 500;

    bool result = bridge->start(config);
    REQUIRE(result);
    REQUIRE(callback->waitForStarted(3000));

    bridge->stop();

    REQUIRE(bridge->state() == ProcessState::Stopped);
    REQUIRE_FALSE(bridge->isRunning());
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
    config.captureStdout = false;
    config.captureStderr = false;

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
    config.captureStdout = false;
    config.captureStderr = false;

    bool result = bridge->start(config);
    REQUIRE(result);

    // Give the process time to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    bridge->stop();
    // No crash = pass
}
