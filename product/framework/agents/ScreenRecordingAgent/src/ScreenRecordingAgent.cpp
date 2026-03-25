#include "ScreenRecordingAgent.h"
#include "LoggerDefine.h"

#include <filesystem>

#include <magic_enum/magic_enum.hpp>

namespace fs = std::filesystem;
using namespace ucf::utilities::screenrecording;

namespace ucf::agents {

// ============================================================================
// Factory
// ============================================================================

std::shared_ptr<IScreenRecordingAgent> IScreenRecordingAgent::create()
{
    return std::make_shared<ScreenRecordingAgent>();
}

// ============================================================================
// Construction / Destruction
// ============================================================================

ScreenRecordingAgent::ScreenRecordingAgent() = default;

ScreenRecordingAgent::~ScreenRecordingAgent()
{
    // Wait for any pending async stop operation
    if (m_stopThread.joinable()) {
        m_stopThread.join();
    }

    // Ensure timer thread is stopped
    stopDurationTimer();

    // If still recording, force stop
    {
        std::lock_guard lock(m_sessionMutex);
        if (m_session.isValid()) {
            ScreenRecordingUtils::stopRecording(m_session);
        }
    }
}

// ============================================================================
// State Machine
// ============================================================================

RecordingAgentState ScreenRecordingAgent::toPublicState(State s)
{
    switch (s) {
    case State::Idle:      return RecordingAgentState::Idle;
    case State::Starting:  return RecordingAgentState::Starting;
    case State::Recording: return RecordingAgentState::Recording;
    case State::Paused:    return RecordingAgentState::Paused;
    case State::Stopping:  return RecordingAgentState::Stopping;
    }
    return RecordingAgentState::Idle;
}

bool ScreenRecordingAgent::tryTransition(State to)
{
    switch (to) {
    case State::Starting:
        if (casFrom({State::Idle}, to)) return true;
        break;
    case State::Recording:
        if (casFrom({State::Starting, State::Paused}, to)) return true;
        break;
    case State::Paused:
        if (casFrom({State::Recording}, to)) return true;
        break;
    case State::Stopping:
        if (casFrom({State::Recording, State::Paused, State::Starting}, to)) return true;
        break;
    case State::Idle:
        if (casFrom({State::Stopping}, to)) return true;
        break;
    }
    SRA_LOG_WARN("Transition rejected: current=" << magic_enum::enum_name(state())
                 << ", target=" << magic_enum::enum_name(to));
    return false;
}

bool ScreenRecordingAgent::casFrom(std::initializer_list<State> fromStates, State to)
{
    for (auto expected : fromStates) {
        if (m_state.compare_exchange_strong(expected, to,
                                            std::memory_order_acq_rel,
                                            std::memory_order_acquire)) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Recording Lifecycle
// ============================================================================

bool ScreenRecordingAgent::start(const RecordingAgentConfig& config)
{
    if (!tryTransition(State::Starting)) {
        fireNotification(&IScreenRecordingAgentCallback::onError,
                         std::string("Recording already in progress"));
        return false;
    }
    fireNotification(&IScreenRecordingAgentCallback::onAgentStateChanged,
                     RecordingAgentState::Starting);

    // Wait for any pending stop operation to complete
    if (m_stopThread.joinable()) {
        m_stopThread.join();
    }

    auto lowLevelConfig = toRecordingConfig(config);
    auto session = ScreenRecordingUtils::startRecording(lowLevelConfig);

    if (!session.isValid()) {
        // Revert state
        m_state.store(State::Idle, std::memory_order_release);
        fireNotification(&IScreenRecordingAgentCallback::onAgentStateChanged,
                         RecordingAgentState::Idle);
        fireNotification(&IScreenRecordingAgentCallback::onError,
                         std::string("Failed to start FFmpeg recording process"));
        return false;
    }

    {
        std::lock_guard lock(m_sessionMutex);
        m_session = session;
    }

    if (!tryTransition(State::Recording)) {
        // Shouldn't happen given we just set Starting, but guard anyway
        std::lock_guard lock(m_sessionMutex);
        ScreenRecordingUtils::stopRecording(m_session);
        m_state.store(State::Idle, std::memory_order_release);
        fireNotification(&IScreenRecordingAgentCallback::onAgentStateChanged,
                         RecordingAgentState::Idle);
        return false;
    }

    startDurationTimer();

    SRA_LOG_INFO("Recording started: " << config.outputPath);
    fireNotification(&IScreenRecordingAgentCallback::onAgentStateChanged,
                     RecordingAgentState::Recording);
    fireNotification(&IScreenRecordingAgentCallback::onRecordingStarted);
    return true;
}

void ScreenRecordingAgent::stop()
{
    if (state() == State::Idle) return;

    // Prevent concurrent stop calls
    bool expected = false;
    if (!m_stopping.compare_exchange_strong(expected, true)) return;

    // Capture whether recording was paused
    bool wasPaused = (state() == State::Paused);

    // Transition to Stopping
    if (!tryTransition(State::Stopping)) {
        m_stopping.store(false);
        return;
    }
    fireNotification(&IScreenRecordingAgentCallback::onAgentStateChanged,
                     RecordingAgentState::Stopping);

    // Join any previous stop thread
    if (m_stopThread.joinable()) {
        m_stopThread.join();
    }

    // Run heavy FFmpeg teardown on a background thread
    m_stopThread = std::thread([this, wasPaused]() {
        stopDurationTimer();

        RecordingResult result;
        {
            std::lock_guard lock(m_sessionMutex);
            if (m_session.isValid()) {
                if (wasPaused) {
                    ScreenRecordingUtils::resumeRecording(m_session);
                }
                result = ScreenRecordingUtils::stopRecording(m_session);
            }
        }

        // Transition back to Idle
        tryTransition(State::Idle);
        fireNotification(&IScreenRecordingAgentCallback::onAgentStateChanged,
                         RecordingAgentState::Idle);

        // If abort was requested, delete the output file silently
        if (m_aborting.exchange(false)) {
            if (!result.outputPath.empty()) {
                std::error_code ec;
                fs::remove(result.outputPath, ec);
            }
            SRA_LOG_INFO("Recording aborted, file deleted");
            fireNotification(&IScreenRecordingAgentCallback::onRecordingAborted);
        } else if (result.success) {
            SRA_LOG_INFO("Recording completed: " << result.outputPath);
            fireNotification(&IScreenRecordingAgentCallback::onRecordingCompleted, result.outputPath);
        } else if (!result.errorMessage.empty()) {
            SRA_LOG_ERROR("Recording error: " << result.errorMessage);
            fireNotification(&IScreenRecordingAgentCallback::onError, result.errorMessage);
        }

        m_stopping.store(false);
    });
}

void ScreenRecordingAgent::abort()
{
    SRA_LOG_INFO("Abort requested");
    m_aborting.store(true);
    stop();
}

void ScreenRecordingAgent::pause()
{
    if (state() != State::Recording) return;

    std::lock_guard lock(m_sessionMutex);
    if (ScreenRecordingUtils::pauseRecording(m_session)) {
        if (tryTransition(State::Paused)) {
            // Stop timer ticking but don't reset
            m_timerRunning.store(false);
            m_timerCv.notify_all();

            SRA_LOG_INFO("Recording paused");
            fireNotification(&IScreenRecordingAgentCallback::onAgentStateChanged,
                             RecordingAgentState::Paused);
            fireNotification(&IScreenRecordingAgentCallback::onRecordingPaused);
        }
    }
}

void ScreenRecordingAgent::resume()
{
    if (state() != State::Paused) return;

    std::lock_guard lock(m_sessionMutex);
    if (ScreenRecordingUtils::resumeRecording(m_session)) {
        if (tryTransition(State::Recording)) {
            // Restart timer ticking
            m_timerRunning.store(true);
            m_timerCv.notify_all();

            SRA_LOG_INFO("Recording resumed");
            fireNotification(&IScreenRecordingAgentCallback::onAgentStateChanged,
                             RecordingAgentState::Recording);
            fireNotification(&IScreenRecordingAgentCallback::onRecordingResumed);
        }
    }
}

// ============================================================================
// State Query
// ============================================================================

RecordingAgentState ScreenRecordingAgent::agentState() const
{
    return toPublicState(state());
}

bool ScreenRecordingAgent::isRecording() const
{
    auto s = state();
    return s == State::Recording || s == State::Paused || s == State::Stopping;
}

bool ScreenRecordingAgent::isPaused() const
{
    return state() == State::Paused;
}

int ScreenRecordingAgent::duration() const
{
    return m_duration.load(std::memory_order_relaxed);
}

// ============================================================================
// Duration Timer
// ============================================================================

void ScreenRecordingAgent::startDurationTimer()
{
    m_duration.store(0);
    m_timerRunning.store(true);
    m_timerShouldExit.store(false);

    // Stop any existing timer first
    if (m_timerThread.joinable()) {
        m_timerShouldExit.store(true);
        m_timerCv.notify_all();
        m_timerThread.join();
        m_timerRunning.store(true);
        m_timerShouldExit.store(false);
    }

    m_timerThread = std::thread([this]() {
        while (true) {
            std::unique_lock lock(m_timerMutex);
            m_timerCv.wait_for(lock, std::chrono::seconds(1), [this]() {
                return m_timerShouldExit.load();
            });

            if (m_timerShouldExit.load()) {
                break;
            }

            // Only count when actively recording (not paused)
            if (m_timerRunning.load()) {
                int seconds = m_duration.fetch_add(1) + 1;
                fireNotification(&IScreenRecordingAgentCallback::onDurationChanged, seconds);
            }
        }
    });
}

void ScreenRecordingAgent::stopDurationTimer()
{
    m_timerShouldExit.store(true);
    m_timerRunning.store(false);
    m_timerCv.notify_all();
    if (m_timerThread.joinable()) {
        m_timerThread.join();
    }
}

// ============================================================================
// Helpers
// ============================================================================

ucf::utilities::screenrecording::RecordingConfig ScreenRecordingAgent::toRecordingConfig(
    const RecordingAgentConfig& cfg)
{
    RecordingConfig rc;
    rc.ffmpegPath = cfg.ffmpegPath;
    rc.outputPath = cfg.outputPath;
    rc.videoFormat = cfg.videoFormat;
    rc.fps = cfg.fps;
    rc.displayIndex = cfg.displayIndex;
    rc.regionX = cfg.regionX;
    rc.regionY = cfg.regionY;
    rc.regionW = cfg.regionW;
    rc.regionH = cfg.regionH;
    rc.isRegion = cfg.isRegion;
    return rc;
}

} // namespace ucf::agents
