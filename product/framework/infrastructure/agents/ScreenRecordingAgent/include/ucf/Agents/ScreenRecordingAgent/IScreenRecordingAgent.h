#pragma once

#include <memory>

#include <ucf/Agents/AgentsCommonFile/AgentsExport.h>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/Agents/ScreenRecordingAgent/ScreenRecordingAgentTypes.h>
#include <ucf/Agents/ScreenRecordingAgent/IScreenRecordingAgentCallback.h>

namespace ucf::agents {

/// High-level orchestration of FFmpeg-based screen recording.
///
/// Manages the FFmpeg child process lifecycle, recording state machine,
/// duration timer, and abort-with-cleanup — all behind a simple interface.
///
/// Internally uses ScreenRecordingUtils for cross-platform FFmpeg process
/// management (stdin pipe for 'q' stop, SuspendThread/SIGSTOP for pause).
///
/// Thread safety:
///   - All public methods are thread-safe.
///   - Callbacks are invoked on internal worker threads.
class Agents_EXPORT IScreenRecordingAgent
    : public virtual ucf::utilities::INotificationHelper<IScreenRecordingAgentCallback>
{
public:
    ~IScreenRecordingAgent() override = default;

    // ── Lifecycle ──

    /// Start an FFmpeg recording subprocess.
    /// @param config  Recording configuration (ffmpeg path, output, region, etc.)
    /// @return true if startup was initiated successfully
    virtual bool start(const RecordingAgentConfig& config) = 0;

    /// Stop the recording gracefully (send 'q' to FFmpeg, wait for exit).
    /// Fires onRecordingCompleted on success.
    /// No-op if not recording.
    virtual void stop() = 0;

    /// Abort the recording: stop FFmpeg and delete the output file.
    /// Fires onRecordingAborted (not onRecordingCompleted).
    /// No-op if not recording.
    virtual void abort() = 0;

    /// Pause the recording (suspend FFmpeg process).
    virtual void pause() = 0;

    /// Resume a paused recording.
    virtual void resume() = 0;

    // ── State Query ──

    /// Current agent state.
    [[nodiscard]] virtual RecordingAgentState agentState() const = 0;

    /// Whether the agent is actively recording (including paused).
    [[nodiscard]] virtual bool isRecording() const = 0;

    /// Whether the recording is currently paused.
    [[nodiscard]] virtual bool isPaused() const = 0;

    /// Current recording duration in seconds.
    [[nodiscard]] virtual int duration() const = 0;

    // ── Audio ──

    /// Enumerate available audio devices (microphones and loopback sources).
    [[nodiscard]] virtual std::vector<AudioDeviceInfo> getAudioDevices() const = 0;

    /// Check if microphone permission is granted (macOS). Always true on other platforms.
    [[nodiscard]] virtual bool hasMicrophonePermission() const = 0;

    /// Request microphone permission. Triggers system dialog on macOS if undetermined.
    /// @param callback  Called asynchronously with the granted result.
    virtual void requestMicrophonePermission(std::function<void(bool)> callback) = 0;

    // ── Factory ──

    /// Create a ScreenRecordingAgent instance.
    static std::shared_ptr<IScreenRecordingAgent> create();
};

} // namespace ucf::agents
