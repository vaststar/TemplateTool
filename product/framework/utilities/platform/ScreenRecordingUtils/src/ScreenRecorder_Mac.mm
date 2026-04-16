#include "ScreenRecorder_Mac.h"

#ifdef __APPLE__

#include "LoggerDefine.h"

#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include <signal.h>



namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers
// ============================================================================

/// Round a dimension down to the nearest even number (minimum 2).
/// libx264 + yuv420p requires even width and height.
static inline int alignToEven(int v)
{
    return std::max(2, v & ~1);
}

// ============================================================================
// Recording — start / stop / pause / resume
// ============================================================================

ScreenRecorder_Mac::ScreenRecorder_Mac() = default;

ScreenRecorder_Mac::~ScreenRecorder_Mac()
{
    if (m_active.load())
    {
        stop();
    }
}

bool ScreenRecorder_Mac::isActive() const
{
    return m_active.load();
}

std::string ScreenRecorder_Mac::outputPath() const
{
    return m_outputPath;
}

bool ScreenRecorder_Mac::start(const RecordingConfig& config)
{
    if (m_active.load())
    {
        SRU_LOG_ERROR("start: already recording");
        return false;
    }

    m_outputPath = config.outputPath;

    if (config.ffmpegPath.empty() || config.outputPath.empty())
    {
        SRU_LOG_ERROR("start: ffmpegPath or outputPath is empty");
        return false;
    }

    // Pre-check screen recording permission to fail fast with a clear error
    // instead of launching an FFmpeg process that silently captures a black screen.
    if (!hasScreenRecordingPermission())
    {
        SRU_LOG_ERROR("start: screen recording permission not granted. "
                      "Go to System Settings > Privacy & Security > Screen Recording to enable.");
        return false;
    }

    // Build FFmpeg command-line arguments
    // Determine audio portion of the input spec
    // For MicAndSystem mode, we need two separate avfoundation inputs
    std::string audioInputName = "none";
    bool needSecondInput = false;
    std::string secondAudioInputName;

    switch (config.audioMode)
    {
    case AudioCaptureMode::None:
        audioInputName = "none";
        break;
    case AudioCaptureMode::Microphone:
        audioInputName = config.micDevice.empty() ? "default" : config.micDevice;
        break;
    case AudioCaptureMode::SystemAudio:
        audioInputName = config.systemAudioDevice.empty() ? "none" : config.systemAudioDevice;
        break;
    case AudioCaptureMode::MicAndSystem:
        // Primary input captures screen + microphone
        audioInputName = config.micDevice.empty() ? "default" : config.micDevice;
        // Second input captures system audio (requires a loopback device like BlackHole)
        if (!config.systemAudioDevice.empty())
        {
            needSecondInput = true;
            secondAudioInputName = config.systemAudioDevice;
        }
        break;
    }

    std::string inputSpec = "Capture screen " + std::to_string(config.displayIndex) + ":" + audioInputName;

    std::vector<std::string> args;
    args.push_back("-y");

    // Increase thread queue to prevent "Thread message queue blocking" drops
    args.insert(args.end(), {"-thread_queue_size", "512",
        "-f", "avfoundation",
        "-framerate", std::to_string(config.fps),
        "-capture_cursor", "1",
        "-use_wallclock_as_timestamps", "1",
        "-probesize", "32",
        "-i", inputSpec});

    // Second avfoundation input for system audio (MicAndSystem mode)
    if (needSecondInput)
    {
        args.insert(args.end(), {
            "-thread_queue_size", "512",
            "-f", "avfoundation",
            "-i", "none:" + secondAudioInputName});
    }

    // Do not synthesize CFR by duplicating frames from unstable input timestamps.
    args.insert(args.end(), {"-vsync", "0"});

    // Region crop filter and audio mixing filter
    std::string videoFilter;
    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        int w = alignToEven(config.regionW);
        int h = alignToEven(config.regionH);
        videoFilter = "crop=" + std::to_string(w) + ":"
                    + std::to_string(h) + ":"
                    + std::to_string(config.regionX) + ":"
                    + std::to_string(config.regionY);
    }

    if (needSecondInput)
    {
        std::string filterComplex;
        if (!videoFilter.empty())
        {
            filterComplex = "[0:v]" + videoFilter + "[vout];"
                "[0:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a0];"
                "[1:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a1];"
                "[a0][a1]amix=inputs=2:duration=first:normalize=0[aout]";
            args.insert(args.end(), {"-filter_complex", filterComplex,
                "-map", "[vout]", "-map", "[aout]"});
        }
        else
        {
            filterComplex =
                "[0:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a0];"
                "[1:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a1];"
                "[a0][a1]amix=inputs=2:duration=first:normalize=0[aout]";
            args.insert(args.end(), {"-filter_complex", filterComplex,
                "-map", "0:v", "-map", "[aout]"});
        }
    }
    else if (!videoFilter.empty())
    {
        args.insert(args.end(), {"-vf", videoFilter});
    }

    // Output codec settings
    if (config.videoFormat == "webm")
    {
        args.insert(args.end(), {"-c:v", "libvpx-vp9", "-b:v", "2M"});
    }
    else
    {
        args.insert(args.end(), {"-c:v", "libx264", "-preset", "ultrafast",
                                 "-tune", "zerolatency", "-pix_fmt", "yuv420p"});
        if (config.videoFormat == "mp4" || config.videoFormat == "mov")
        {
            args.insert(args.end(), {"-video_track_timescale", "600"});
        }
    }

    // Audio codec settings (only when audio is enabled)
    if (config.audioMode != AudioCaptureMode::None)
    {
        args.insert(args.end(), {"-ar", "48000", "-ac", "2"});
        if (config.videoFormat == "webm")
        {
            args.insert(args.end(), {"-c:a", "libopus", "-b:a", "128k"});
        }
        else
        {
            args.insert(args.end(), {"-c:a", "aac", "-b:a", "128k"});
        }
    }

    args.push_back(config.outputPath);

    // ── Launch FFmpeg via ProcessBridge ──
    ucf::utilities::ProcessBridgeConfig pbConfig;
    pbConfig.executablePath = config.ffmpegPath;
    pbConfig.arguments = std::move(args);
    pbConfig.pipeStdin = true;
    pbConfig.captureStdout = false;
    pbConfig.captureStderr = true;
    pbConfig.stopTimeoutMs = 10000;

    m_process = ucf::utilities::IProcessBridge::create();
    if (!m_process->start(pbConfig))
    {
        SRU_LOG_ERROR("start: ProcessBridge failed to launch FFmpeg");
        m_process.reset();
        return false;
    }

    m_active.store(true);

    SRU_LOG_INFO("start: started ffmpeg pid=" << m_process->processPid()
                 << " output=" << config.outputPath);
    return true;
}

RecordingResult ScreenRecorder_Mac::stop()
{
    RecordingResult result;
    result.outputPath = m_outputPath;

    if (!m_active.load())
    {
        result.errorMessage = "Not recording";
        return result;
    }

    // Send 'q' to FFmpeg's stdin to gracefully stop.
    if (m_process)
    {
        m_process->writeToStdin("q\n");
        m_process->closeStdin();
    }

    // Wait for FFmpeg to exit naturally after receiving 'q'.
    if (m_process)
    {
        constexpr int kTimeoutMs = 10000;
        constexpr int kPollMs    = 100;
        int waited = 0;
        while (m_process->isRunning() && waited < kTimeoutMs)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(kPollMs));
            waited += kPollMs;
        }

        if (m_process->isRunning())
        {
            SRU_LOG_WARN("stop: FFmpeg did not exit within "
                         << kTimeoutMs << "ms, force-killing");
            m_process->stop();
            result.errorMessage = "FFmpeg did not exit in time, force killed";
        }

        m_process.reset();
    }

    m_active.store(false);

    // Check if output file exists
    std::error_code ec;
    if (std::filesystem::is_regular_file(result.outputPath, ec))
    {
        result.success = true;
    }
    else
    {
        if (result.errorMessage.empty())
        {
            result.errorMessage = "Output file not found: " + result.outputPath;
        }
        SRU_LOG_ERROR("stop: " << result.errorMessage);
    }

    return result;
}

bool ScreenRecorder_Mac::pause()
{
    if (!m_active.load() || !m_process)
    {
        return false;
    }
    return kill(static_cast<pid_t>(m_process->processPid()), SIGSTOP) == 0;
}

bool ScreenRecorder_Mac::resume()
{
    if (!m_active.load() || !m_process)
    {
        return false;
    }
    return kill(static_cast<pid_t>(m_process->processPid()), SIGCONT) == 0;
}

} // namespace ucf::utilities::screenrecording

#endif // __APPLE__
