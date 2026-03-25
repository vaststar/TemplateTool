#pragma once

#include <atomic>
#include <initializer_list>
#include <mutex>
#include <thread>

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>
#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridgeCallback.h>
#include <ucf/Utilities/ProcessBridgeUtils/ProcessBridgeConfig.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include "ProcessLauncher.h"

namespace ucf::utilities {

class ProcessBridge final : public virtual IProcessBridge, public virtual NotificationHelper<IProcessBridgeCallback>
{
public:
    ProcessBridge() = default;
    ~ProcessBridge() override;

    ProcessBridge(const ProcessBridge&) = delete;
    ProcessBridge& operator=(const ProcessBridge&) = delete;
    ProcessBridge(ProcessBridge&&) = delete;
    ProcessBridge& operator=(ProcessBridge&&) = delete;

    // â”€â”€ IProcessBridge â”€â”€
    bool start(const ProcessBridgeConfig& config) override;
    void stop() override;
    ProcessState state() const override;
    bool isRunning() const override;
    int64_t processPid() const override;

private:
    // â”€â”€ State machine â”€â”€

    /// Attempt to transition mState to @p to.
    /// Valid source states are defined internally per target state.
    /// Logs the transition result (success or rejection with current state).
    /// @return true if the transition was performed
    bool tryTransition(ProcessState to);

    /// CAS helper: try transitioning from any of @p fromStates to @p to.
    /// @return true if any CAS succeeded
    bool casFrom(std::initializer_list<ProcessState> fromStates, ProcessState to);

    /// Attempt to acquire stop ownership by transitioning to Stopping.
    /// @return true if this caller owns shutdown/finalization
    bool beginStop();

    /// Finalize a startup failure before Running was reached.
    void failStart(const std::string& errorMessage);

    // â”€â”€ Process management â”€â”€

    /// Background loop that monitors the child process.
    void monitorLoop();

    /// Read available pipe data and fire callbacks.
    void readAndFirePipes();

    /// Graceful terminate â†’ waitForExit â†’ force kill if needed. Returns exit code.
    int terminateAndWait();

    /// Close handles, drain pipes, fire onProcessStopped, transition to Terminated.
    /// @param exitCode   Child process exit code
    /// @param crashed    true if force-killed or non-zero exit
    void finalizeStop(int exitCode, bool crashed);

    std::atomic<ProcessState> mState{ProcessState::Idle};
    std::atomic<int64_t> mPid{0};

    /// Guards mConfig/mHandle/mMonitorThread writes during start() setup.
    /// stop() acquires briefly as a sync barrier to ensure setup is complete.
    std::mutex mLifecycleMutex;

    ProcessBridgeConfig mConfig;
    detail::ProcessLauncher::ProcessHandle mHandle;

    std::thread mMonitorThread;

    static constexpr int kPollIntervalMs = 50;
};

} // namespace ucf::utilities
