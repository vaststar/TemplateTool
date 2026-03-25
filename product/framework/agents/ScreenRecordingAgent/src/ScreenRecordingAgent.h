#pragma once

#include <ucf/Agents/ScreenRecordingAgent/IScreenRecordingAgent.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace ucf::agents {

/// Private implementation of IScreenRecordingAgent.
///
/// Owns the FFmpeg child-process lifecycle, a duration timer thread,
/// and an asynchronous stop thread.
class ScreenRecordingAgent final
    : public virtual IScreenRecordingAgent
    , public virtual ucf::utilities::NotificationHelper<IScreenRecordingAgentCallback>
{
public:
    ScreenRecordingAgent();
    ~ScreenRecordingAgent() override;

    // ── IScreenRecordingAgent ──
    bool start(const RecordingAgentConfig& config) override;
    void stop() override;
    void abort() override;
    void pause() override;
    void resume() override;

    [[nodiscard]] RecordingAgentState agentState() const override;
    [[nodiscard]] bool isRecording() const override;
    [[nodiscard]] bool isPaused() const override;
    [[nodiscard]] int duration() const override;

private:
    /// Internal state machine.
    enum class State { Idle, Starting, Recording, Paused, Stopping };

    /// Map internal state to public enum.
    static RecordingAgentState toPublicState(State s);

    /// Attempt a state transition; returns false if not allowed.
    bool tryTransition(State to);

    /// Compare-and-swap helper for state transitions.
    bool casFrom(std::initializer_list<State> fromStates, State to);

    /// Current state (atomic for lock-free reads).
    [[nodiscard]] State state() const { return m_state.load(std::memory_order_acquire); }

    // ── Duration timer ──
    void startDurationTimer();
    void stopDurationTimer();

    /// Convert RecordingAgentConfig to the low-level RecordingConfig.
    static ucf::utilities::screenrecording::RecordingConfig toRecordingConfig(
        const RecordingAgentConfig& cfg);

private:
    std::atomic<State> m_state{State::Idle};

    // Active FFmpeg session
    mutable std::mutex m_sessionMutex;
    ucf::utilities::screenrecording::RecordingSession m_session;

    // Duration timer
    std::atomic<int> m_duration{0};
    std::atomic<bool> m_timerRunning{false};
    std::atomic<bool> m_timerShouldExit{false};
    std::thread m_timerThread;
    std::condition_variable m_timerCv;
    std::mutex m_timerMutex;

    // Async stop / abort
    std::atomic<bool> m_stopping{false};
    std::atomic<bool> m_aborting{false};
    std::thread m_stopThread;
};

} // namespace ucf::agents
