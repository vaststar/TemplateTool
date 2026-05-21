#pragma once

#include <string>

#include <ucf/Agents/ScreenRecordingAgent/ScreenRecordingAgentTypes.h>

namespace ucf::agents {

/// Callback interface for ScreenRecordingAgent events.
///
/// All callbacks are invoked on internal worker threads.
/// Implementations must ensure thread safety.
class IScreenRecordingAgentCallback
{
public:
    virtual ~IScreenRecordingAgentCallback() = default;

    /// Agent state changed.
    virtual void onAgentStateChanged(RecordingAgentState state) = 0;

    /// Recording has started (FFmpeg process is live).
    virtual void onRecordingStarted() = 0;

    /// Recording has been paused.
    virtual void onRecordingPaused() = 0;

    /// Recording has been resumed from pause.
    virtual void onRecordingResumed() = 0;

    /// Duration tick (seconds elapsed since recording start).
    virtual void onDurationChanged(int seconds) = 0;

    /// Recording completed normally — output file is valid.
    virtual void onRecordingCompleted(const std::string& outputPath) = 0;

    /// Recording was aborted — output file has been deleted.
    virtual void onRecordingAborted() = 0;

    /// An error occurred.
    virtual void onError(const std::string& message) = 0;
};

} // namespace ucf::agents
