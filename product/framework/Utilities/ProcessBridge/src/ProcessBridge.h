#pragma once

#include <atomic>
#include <mutex>
#include <thread>

#include <ucf/Utilities/ProcessBridge/IProcessBridge.h>
#include <ucf/Utilities/ProcessBridge/IProcessBridgeCallback.h>
#include <ucf/Utilities/ProcessBridge/ProcessBridgeConfig.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include "ProcessLauncher.h"

namespace ucf::utilities {

/// Concrete implementation of IProcessBridge.
///
/// Manages a child process lifecycle on a background monitor thread.
/// The monitor thread polls process liveness and pipes (stdout/stderr),
/// firing callbacks via NotificationHelper.
///
/// Thread safety:
///   - start() / stop() are guarded by CAS on mState.
///   - stop() from within a callback (monitor thread) is safe — cleanup
///     is deferred to the monitorLoop exit path.
///   - Only mState transitions decide "who cleans up"; no separate
///     mExitHandled flag is needed.
class ProcessBridge final
    : public virtual NotificationHelper<IProcessBridgeCallback>
    , public IProcessBridge
{
public:
    ProcessBridge() = default;
    ~ProcessBridge() override;

    ProcessBridge(const ProcessBridge&) = delete;
    ProcessBridge& operator=(const ProcessBridge&) = delete;
    ProcessBridge(ProcessBridge&&) = delete;
    ProcessBridge& operator=(ProcessBridge&&) = delete;

    // ── IProcessBridge ──
    bool start(const ProcessBridgeConfig& config) override;
    void stop() override;
    ProcessState state() const override;
    bool isRunning() const override;
    int64_t processPid() const override;

private:
    /// Background loop that monitors the child process.
    void monitorLoop();

    /// Read available pipe data and fire callbacks.
    void readAndFirePipes();

    /// Close handles, drain pipes, fire onProcessStopped, transition to terminal state.
    /// @param exitCode   Child process exit code
    /// @param crashed    true if force-killed or non-zero exit
    void cleanupProcess(int exitCode, bool crashed);

    std::atomic<ProcessState> mState{ProcessState::Idle};
    std::atomic<int64_t> mPid{0};
    std::atomic<bool> mStopRequested{false};

    ProcessBridgeConfig mConfig;
    detail::ProcessLauncher::ProcessHandle mHandle;

    std::thread mMonitorThread;

    static constexpr int kPollIntervalMs = 50;
};

} // namespace ucf::utilities
