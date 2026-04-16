#include "ProcessBridge.h"
#include "ProcessBridgeLogger.h"

#include <chrono>
#include <magic_enum/magic_enum.hpp>

namespace ucf::utilities {

// ════════════════════════════════════════════════════════════
//  Factory
// ════════════════════════════════════════════════════════════

std::unique_ptr<IProcessBridge> IProcessBridge::create()
{
    return std::make_unique<ProcessBridge>();
}

// ════════════════════════════════════════════════════════════
//  State machine
// ════════════════════════════════════════════════════════════

bool ProcessBridge::tryTransition(ProcessState to)
{
    switch (to)
    {
    case ProcessState::Starting:
        if (casFrom({ProcessState::Idle, ProcessState::Terminated}, to))
        {
            return true;
        }
        break;

    case ProcessState::Running:
        if (casFrom({ProcessState::Starting}, to))
        {
            return true;
        }
        break;

    case ProcessState::Stopping:
        if (casFrom({ProcessState::Starting, ProcessState::Running}, to))
        {
            return true;
        }
        break;

    case ProcessState::Terminated:
        if (casFrom({ProcessState::Starting, ProcessState::Stopping}, to))
        {
            return true;
        }
        break;

    case ProcessState::Idle:
        break;  // Idle is the initial state, no transition back
    }

    PB_LOG_DEBUG("Transition to " << magic_enum::enum_name(to)
                 << " rejected, current=" << magic_enum::enum_name(state()));
    return false;
}

bool ProcessBridge::casFrom(std::initializer_list<ProcessState> fromStates,
                            ProcessState to)
{
    for (auto from : fromStates)
    {
        auto expected = from;
        if (mState.compare_exchange_strong(expected, to,
                                           std::memory_order_acq_rel,
                                           std::memory_order_acquire))
        {
            PB_LOG_DEBUG("State: " << magic_enum::enum_name(from)
                         << " -> " << magic_enum::enum_name(to));
            return true;
        }
    }
    return false;
}

bool ProcessBridge::beginStop()
{
    if (!tryTransition(ProcessState::Stopping))
    {
        PB_LOG_DEBUG("Stop skipped, state=" << magic_enum::enum_name(state()));
        return false;
    }

    PB_LOG_INFO("Stop begin, state=" << magic_enum::enum_name(state()));
    return true;
}

void ProcessBridge::failStart(const std::string& errorMessage)
{
    mPid = 0;
    tryTransition(ProcessState::Terminated);
    fireNotification(&IProcessBridgeCallback::onProcessError, errorMessage);
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
    if (!tryTransition(ProcessState::Starting))
    {
        PB_LOG_WARN("start() rejected, state=" << magic_enum::enum_name(state()));
        return false;
    }

    // Clean up previous run's monitor thread (may still be joinable after
    // a natural exit where monitorLoop returned without being joined)
    if (mMonitorThread.joinable())
    {
        mMonitorThread.join();
    }

    // Lock scope: protect shared data writes so that stop() cannot read
    // mHandle/mMonitorThread in a partially-written state.
    bool launchFailed = false;
    std::string errorMsg;
    {
        std::lock_guard<std::mutex> lock(mLifecycleMutex);

        mConfig = config;

        PB_LOG_INFO("Launching: " << config.executablePath);

        auto handle = detail::ProcessLauncher::launch(
            config.executablePath,
            config.arguments,
            config.workingDirectory,
            config.pipeStdin,
            config.environment);

        if (!handle.valid)
        {
            PB_LOG_ERROR("Failed to launch: " << handle.errorMessage);
            mPid = 0;
            errorMsg = handle.errorMessage;
            launchFailed = true;
        }
        else
        {
            mHandle = handle;
            mPid = handle.pid;

            // Start monitor thread BEFORE setting Running —
            // ensures the thread object is valid when stop() tries to join
            mMonitorThread = std::thread(&ProcessBridge::monitorLoop, this);
        }
    }
    // Lock released — stop() can now safely access mHandle/mMonitorThread

    if (launchFailed)
    {
        failStart(errorMsg);
        return false;
    }

    if (!tryTransition(ProcessState::Running))
    {
        // stop() has intervened (Starting → Stopping) while we were launching.
        // The monitor thread will handle cleanup via its exit path.
        PB_LOG_WARN("start() interrupted by concurrent stop(), state="
                    << magic_enum::enum_name(state()));
        return false;
    }

    PB_LOG_INFO("Process started, PID=" << mPid.load());
    fireNotification(&IProcessBridgeCallback::onProcessStarted, mPid.load());

    return true;
}

void ProcessBridge::stop()
{
    if (!beginStop())
    {
        return;
    }

    PB_LOG_INFO("Stopping process PID=" << mPid.load());

    // Sync barrier: wait for start() setup to complete (if it's still
    // inside the locked region writing mHandle/mMonitorThread).
    {
        std::lock_guard<std::mutex> lock(mLifecycleMutex);
    }

    // Launch may have failed before a valid process handle existed.
    if (!mHandle.valid)
    {
        tryTransition(ProcessState::Terminated);
        return;
    }

    // If called from within a callback on the monitor thread, we cannot
    // join ourselves. The monitorLoop exit path will do deferred cleanup.
    if (std::this_thread::get_id() == mMonitorThread.get_id())
    {
        PB_LOG_INFO("stop() called from monitor thread, cleanup deferred to monitorLoop exit");
        return;
    }

    if (mMonitorThread.joinable())
    {
        mMonitorThread.join();
    }

    // After join, monitorLoop should have called finalizeStop (→ Terminated).
    // Safety net: if monitorLoop exited without finalizing, do it here.
    if (state() == ProcessState::Stopping)
    {
        PB_LOG_WARN("monitorLoop did not finalize, cleaning up in stop()");
        int exitCode = terminateAndWait();
        finalizeStop(exitCode, false);
    }
}

// ════════════════════════════════════════════════════════════
//  State queries
// ════════════════════════════════════════════════════════════

ProcessState ProcessBridge::state() const
{
    return mState.load(std::memory_order_acquire);
}

bool ProcessBridge::isRunning() const
{
    auto s = state();
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
//  Stdin
// ════════════════════════════════════════════════════════════

bool ProcessBridge::writeToStdin(const std::string& data)
{
    if (!isRunning() || !mHandle.valid)
    {
        PB_LOG_WARN("writeToStdin: process not running");
        return false;
    }
    return detail::ProcessLauncher::writeStdin(mHandle, data);
}

void ProcessBridge::closeStdin()
{
    detail::ProcessLauncher::closeStdin(mHandle);
}

// ════════════════════════════════════════════════════════════
//  Monitor thread
// ════════════════════════════════════════════════════════════

void ProcessBridge::monitorLoop()
{
    PB_LOG_DEBUG("Monitor thread started for PID=" << mPid.load());

    bool naturalExit = false;

    while (true)
    {
        auto currentSt = state();
        if (currentSt == ProcessState::Stopping || currentSt == ProcessState::Terminated)
        {
            PB_LOG_DEBUG("Monitor thread detected stop request, exiting loop (state="
                         << magic_enum::enum_name(currentSt) << ")");
            break;
        }

        readAndFirePipes();

        if (!detail::ProcessLauncher::isAlive(mHandle))
        {
            PB_LOG_INFO("Process PID=" << mPid.load() << " exited on its own");
            // Transition to Stopping so we own cleanup.
            // Only mark naturalExit if WE won the transition — if stop() already
            // set Stopping (and killed the process), it's not a natural exit.
            if (beginStop())
            {
                naturalExit = true;
            }
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(kPollIntervalMs));
    }

    // Unified cleanup: handles both natural exit and deferred stop
    // (stop() called from callback on this thread set Stopping but could not join/cleanup)
    if (state() == ProcessState::Stopping)
    {
        PB_LOG_INFO(naturalExit ? "Cleanup after natural process exit" : "Deferred cleanup in monitorLoop exit");

        // Deferred stop: process may still be alive (stop() only set Stopping
        // but couldn't terminate because it returned early to avoid self-join).
        if (!naturalExit && detail::ProcessLauncher::isAlive(mHandle))
        {
            int exitCode = terminateAndWait();
            finalizeStop(exitCode, false);
        }
        else
        {
            int exitCode = detail::ProcessLauncher::waitForExit(mHandle, 0);
            finalizeStop(exitCode, naturalExit && exitCode != 0);
        }
    }
}

void ProcessBridge::readAndFirePipes()
{
    // Always drain pipes to prevent child from blocking on a full pipe buffer,
    // but only fire callbacks when capture is enabled.
    if (auto stdoutData = detail::ProcessLauncher::readStdout(mHandle); !stdoutData.empty())
    {
        if (mConfig.captureStdout)
        {
            fireNotification(&IProcessBridgeCallback::onStdout, stdoutData);
        }
    }

    if (auto stderrData = detail::ProcessLauncher::readStderr(mHandle); !stderrData.empty())
    {
        if (mConfig.captureStderr)
        {
            fireNotification(&IProcessBridgeCallback::onStderr, stderrData);
        }
    }
}

int ProcessBridge::terminateAndWait()
{
    detail::ProcessLauncher::terminate(mHandle);
    int exitCode = detail::ProcessLauncher::waitForExit(mHandle, mConfig.stopTimeoutMs);

    if (exitCode == -1 && detail::ProcessLauncher::isAlive(mHandle))
    {
        PB_LOG_WARN("Graceful stop timed out, force killing PID=" << mPid.load());
        detail::ProcessLauncher::kill(mHandle);
        exitCode = detail::ProcessLauncher::waitForExit(mHandle, 2000);
    }
    return exitCode;
}

void ProcessBridge::finalizeStop(int exitCode, bool crashed)
{
    readAndFirePipes();
    detail::ProcessLauncher::closeHandles(mHandle);
    mPid = 0;
    tryTransition(ProcessState::Terminated);

    PB_LOG_INFO("Process finalized, exitCode=" << exitCode << ", crashed=" << crashed);
    fireNotification(&IProcessBridgeCallback::onProcessStopped, exitCode, crashed);
}

// ════════════════════════════════════════════════════════════
//  Static synchronous run
// ════════════════════════════════════════════════════════════

IProcessBridge::RunResult IProcessBridge::run(const ProcessBridgeConfig& config)
{
    RunResult result;

    auto handle = detail::ProcessLauncher::launch(
        config.executablePath,
        config.arguments,
        config.workingDirectory,
        config.pipeStdin,
        config.environment);

    if (!handle.valid)
    {
        result.exitCode = -1;
        result.stderrData = handle.errorMessage;
        return result;
    }

    // Poll for output and process exit
    int timeoutMs = config.stopTimeoutMs > 0 ? config.stopTimeoutMs : 30000;
    auto deadline = std::chrono::steady_clock::now()
                  + std::chrono::milliseconds(timeoutMs);
    constexpr auto pollInterval = std::chrono::milliseconds(50);

    while (std::chrono::steady_clock::now() < deadline)
    {
        // Always drain pipes to prevent child from blocking on a full pipe buffer.
        if (auto stdoutChunk = detail::ProcessLauncher::readStdout(handle); !stdoutChunk.empty() && config.captureStdout)
        {
            result.stdoutData += stdoutChunk;
        }
        if (auto stderrChunk = detail::ProcessLauncher::readStderr(handle); !stderrChunk.empty() && config.captureStderr)
        {
            result.stderrData += stderrChunk;
        }

        if (!detail::ProcessLauncher::isAlive(handle))
        {
            break;
        }

        std::this_thread::sleep_for(pollInterval);
    }

    // Drain remaining output
    if (auto finalStdout = detail::ProcessLauncher::readStdout(handle); !finalStdout.empty() && config.captureStdout)
    {
        result.stdoutData += finalStdout;
    }
    if (auto finalStderr = detail::ProcessLauncher::readStderr(handle); !finalStderr.empty() && config.captureStderr)
    {
        result.stderrData += finalStderr;
    }

    if (detail::ProcessLauncher::isAlive(handle))
    {
        // Timed out — force kill
        result.timedOut = true;
        detail::ProcessLauncher::terminate(handle);
        int code = detail::ProcessLauncher::waitForExit(handle, 3000);
        if (code == -1 && detail::ProcessLauncher::isAlive(handle))
        {
            detail::ProcessLauncher::kill(handle);
            code = detail::ProcessLauncher::waitForExit(handle, 2000);
        }
        result.exitCode = code;
    }
    else
    {
        result.exitCode = detail::ProcessLauncher::waitForExit(handle, 0);
    }

    detail::ProcessLauncher::closeHandles(handle);
    return result;
}

} // namespace ucf::utilities
