#include "ProcessBridge.h"
#include "ProcessBridgeLogger.h"

#include <chrono>

namespace ucf::utilities {

// ════════════════════════════════════════════════════════════
//  Factory
// ════════════════════════════════════════════════════════════

std::unique_ptr<IProcessBridge> IProcessBridge::create()
{
    return std::make_unique<ProcessBridge>();
}

// ════════════════════════════════════════════════════════════
//  Lifecycle
// ════════════════════════════════════════════════════════════

ProcessBridge::~ProcessBridge()
{
    stop();
    // Ensure the monitor thread is joined even if the process exited
    // naturally (monitorLoop returned on its own without stop() joining it).
    if (mMonitorThread.joinable())
    {
        mMonitorThread.join();
    }
}

bool ProcessBridge::start(const ProcessBridgeConfig& config)
{
    // Only allow start from a terminal state
    auto expected = mState.load();
    if (expected != ProcessState::Idle && expected != ProcessState::Terminated)
    {
        PB_LOG_WARN("start() rejected, state=" << static_cast<int>(expected));
        return false;
    }
    // CAS: only one caller can win the transition to Starting
    if (!mState.compare_exchange_strong(expected, ProcessState::Starting))
    {
        PB_LOG_WARN("start() rejected (concurrent), state=" << static_cast<int>(expected));
        return false;
    }

    // Clean up previous run's monitor thread (may still be joinable after
    // a natural exit where monitorLoop returned without being joined)
    if (mMonitorThread.joinable())
    {
        mMonitorThread.join();
    }

    mConfig = config;
    mStopRequested = false;

    PB_LOG_INFO("Launching: " << config.executablePath);

    auto handle = detail::ProcessLauncher::launch(
        config.executablePath,
        config.arguments,
        config.workingDirectory);

    if (!handle.valid)
    {
        PB_LOG_ERROR("Failed to launch: " << handle.errorMessage);
        mPid = 0;
        mState = ProcessState::Terminated;
        fireNotification(&IProcessBridgeCallback::onProcessError, handle.errorMessage);
        return false;
    }

    mHandle = handle;
    mPid = handle.pid;

    // Start monitor thread BEFORE setting Running —
    // ensures the thread object is valid when stop() tries to join
    mMonitorThread = std::thread(&ProcessBridge::monitorLoop, this);
    mState = ProcessState::Running;

    PB_LOG_INFO("Process started, PID=" << handle.pid);
    fireNotification(&IProcessBridgeCallback::onProcessStarted, handle.pid);

    return true;
}

void ProcessBridge::stop()
{
    // Only allow stop from an active state
    auto expected = mState.load();
    if (expected != ProcessState::Running && expected != ProcessState::Starting)
    {
        PB_LOG_DEBUG("stop() skipped, state=" << static_cast<int>(expected));
        return;
    }
    // CAS: only one caller can win the transition to Stopping
    if (!mState.compare_exchange_strong(expected, ProcessState::Stopping))
    {
        PB_LOG_DEBUG("stop() skipped (concurrent), state=" << static_cast<int>(expected));
        return;
    }

    mStopRequested = true;
    PB_LOG_INFO("Stopping process PID=" << mPid.load());

    // Graceful terminate
    detail::ProcessLauncher::terminate(mHandle);
    int exitCode = detail::ProcessLauncher::waitForExit(mHandle, mConfig.stopTimeoutMs);

    // Force kill if still alive
    if (exitCode == -1 && detail::ProcessLauncher::isAlive(mHandle))
    {
        PB_LOG_WARN("Graceful stop timed out, force killing PID=" << mPid.load());
        detail::ProcessLauncher::kill(mHandle);
        exitCode = detail::ProcessLauncher::waitForExit(mHandle, 2000);
    }

    // Join monitor thread
    if (mMonitorThread.joinable())
    {
        mMonitorThread.join();
    }

    if (mState.load() != ProcessState::Terminated)
    {
        cleanupProcess(exitCode, false);
    }
}

// ════════════════════════════════════════════════════════════
//  State queries
// ════════════════════════════════════════════════════════════

ProcessState ProcessBridge::state() const
{
    return mState.load();
}

bool ProcessBridge::isRunning() const
{
    auto s = mState.load();
    return s == ProcessState::Starting || s == ProcessState::Running;
}

int64_t ProcessBridge::processPid() const
{
    if (isRunning())
    {
        return mPid.load();
    }
    return 0;
}

// ════════════════════════════════════════════════════════════
//  Monitor thread
// ════════════════════════════════════════════════════════════

void ProcessBridge::monitorLoop()
{
    PB_LOG_DEBUG("Monitor thread started for PID=" << mPid.load());

    while (!mStopRequested.load())
    {
        readAndFirePipes();

        if (!detail::ProcessLauncher::isAlive(mHandle))
        {
            PB_LOG_INFO("Process PID=" << mPid.load() << " exited on its own");
            int exitCode = detail::ProcessLauncher::waitForExit(mHandle, 0);

            // Transition Running → Stopped/Error directly (natural exit)
            auto exp = ProcessState::Running;
            if (mState.compare_exchange_strong(exp, ProcessState::Stopping))
            {
                cleanupProcess(exitCode, exitCode != 0);
            }
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(kPollIntervalMs));
    }

    PB_LOG_DEBUG("Monitor thread exiting (stop requested)");
}

void ProcessBridge::readAndFirePipes()
{
    if (auto stdoutData = detail::ProcessLauncher::readStdout(mHandle); !stdoutData.empty())
    {
        fireNotification(&IProcessBridgeCallback::onStdout, stdoutData);
    }

    if (auto stderrData = detail::ProcessLauncher::readStderr(mHandle); !stderrData.empty())
    {
        fireNotification(&IProcessBridgeCallback::onStderr, stderrData);
    }
}

void ProcessBridge::cleanupProcess(int exitCode, bool crashed)
{
    mState = ProcessState::Terminated;
    readAndFirePipes();
    detail::ProcessLauncher::closeHandles(mHandle);
    mPid = 0;

    PB_LOG_INFO("Process finalized, exitCode=" << exitCode << ", crashed=" << crashed);
    fireNotification(&IProcessBridgeCallback::onProcessStopped, exitCode, crashed);
}

} // namespace ucf::utilities
