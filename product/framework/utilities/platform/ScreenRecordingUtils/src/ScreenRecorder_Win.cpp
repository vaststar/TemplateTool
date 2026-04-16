#include "ScreenRecorder_Win.h"

#ifdef _WIN32

#include "LoggerDefine.h"
#include "WasapiLoopbackCapture.h"

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridgeCallback.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tlhelp32.h>

namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers
// ============================================================================

/// Round a dimension down to the nearest even number (minimum 2).
static inline int alignToEven(int v)
{
    return (std::max)(2, v & ~1);
}

/// Escape special characters in a DirectShow device name for FFmpeg.
static std::string escapeDshowDeviceName(const std::string& name)
{
    std::string escaped;
    escaped.reserve(name.size() + 8);
    for (char c : name)
    {
        if (c == '\\' || c == '\'' || c == ';' || c == '=' || c == ':')
        {
            escaped += '\\';
        }
        escaped += c;
    }
    return escaped;
}

// ============================================================================
// ProcessBridge callback — logs FFmpeg stderr and exit status
// ============================================================================

class FFmpegProcessCallback final : public ucf::utilities::IProcessBridgeCallback
{
public:
    void onProcessStarted(int64_t pid) override
    {
        SRU_LOG_INFO("FFmpeg started, pid=" << pid);
    }
    void onProcessStopped(int exitCode, bool crashed) override
    {
        if (exitCode == 0 && !crashed)
        {
            SRU_LOG_INFO("FFmpeg exited normally");
        }
        else
        {
            SRU_LOG_WARN("FFmpeg exited: code=" << exitCode << " crashed=" << crashed);
        }
    }
    void onProcessError(const std::string& msg) override
    {
        SRU_LOG_ERROR("FFmpeg process error: " << msg);
    }
    void onStderr(const std::string& data) override
    {
        SRU_LOG_DEBUG("FFmpeg stderr: " << data);
    }
};

// ============================================================================
// Construction / Destruction
// ============================================================================

ScreenRecorder_Win::ScreenRecorder_Win() = default;

ScreenRecorder_Win::~ScreenRecorder_Win()
{
    if (m_active.load())
    {
        stop();
    }
}

// ============================================================================
// IScreenRecorder — start
// ============================================================================

bool ScreenRecorder_Win::start(const RecordingConfig& config)
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

    // ── WASAPI loopback pipe (if system audio via OutputDevice) ──
    bool useWasapiLoopback = false;
    std::string namedPipePath;
    HANDLE hNamedPipeServer = INVALID_HANDLE_VALUE;

    bool needSystemAudio = (config.audioMode == AudioCaptureMode::SystemAudio ||
                            config.audioMode == AudioCaptureMode::MicAndSystem);

    if (needSystemAudio && config.systemAudioDeviceType == AudioDeviceType::OutputDevice)
    {
        namedPipePath = "\\\\.\\pipe\\sru_loopback_" + std::to_string(GetCurrentProcessId())
                      + "_" + std::to_string(GetTickCount64());

        hNamedPipeServer = CreateNamedPipeA(
            namedPipePath.c_str(),
            PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1, 65536, 0, 0, nullptr);

        if (hNamedPipeServer == INVALID_HANDLE_VALUE)
        {
            SRU_LOG_ERROR("start: CreateNamedPipe failed, error=" << GetLastError());
            return false;
        }

        m_loopbackCapture = std::make_unique<WasapiLoopbackCapture>();
        if (!m_loopbackCapture->probeFormat(config.systemAudioDevice))
        {
            SRU_LOG_ERROR("start: WasapiLoopbackCapture failed to probe device format");
            CloseHandle(hNamedPipeServer);
            m_loopbackCapture.reset();
            return false;
        }

        useWasapiLoopback = true;
    }

    // ── Build FFmpeg arguments ──
    std::vector<std::string> args;
    args.insert(args.end(), {"-y",
        "-thread_queue_size", "512",
        "-probesize", "5M",
        "-f", "gdigrab",
        "-framerate", std::to_string(config.fps),
        "-draw_mouse", "1"});

    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        int w = alignToEven(config.regionW);
        int h = alignToEven(config.regionH);
        args.insert(args.end(), {
            "-offset_x", std::to_string(config.regionX),
            "-offset_y", std::to_string(config.regionY),
            "-video_size", std::to_string(w) + "x" + std::to_string(h)});
    }

    args.insert(args.end(), {"-i", "desktop"});

    // ── Audio inputs ──
    bool hasAudio = (config.audioMode != AudioCaptureMode::None);
    bool needAudioMix = false;

    if (config.audioMode == AudioCaptureMode::Microphone ||
        config.audioMode == AudioCaptureMode::MicAndSystem)
    {
        std::string micName = config.micDevice.empty()
            ? "Microphone" : escapeDshowDeviceName(config.micDevice);
        args.insert(args.end(), {
            "-thread_queue_size", "512",
            "-f", "dshow",
            "-i", "audio=" + micName});
    }

    if (needSystemAudio)
    {
        if (useWasapiLoopback)
        {
            args.insert(args.end(), {
                "-use_wallclock_as_timestamps", "1",
                "-thread_queue_size", "512",
                "-f", "s16le",
                "-ar", std::to_string(m_loopbackCapture->sampleRate()),
                "-ac", std::to_string(m_loopbackCapture->channels()),
                "-i", namedPipePath});
        }
        else
        {
            std::string sysName = config.systemAudioDevice.empty()
                ? "Stereo Mix" : escapeDshowDeviceName(config.systemAudioDevice);
            args.insert(args.end(), {
                "-thread_queue_size", "512",
                "-f", "dshow",
                "-i", "audio=" + sysName});
        }

        if (config.audioMode == AudioCaptureMode::MicAndSystem)
        {
            needAudioMix = true;
        }
    }

    if (needAudioMix)
    {
        args.insert(args.end(), {
            "-filter_complex",
            "[1:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo,asetpts=PTS-STARTPTS[a0];"
            "[2:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo,asetpts=PTS-STARTPTS[a1];"
            "[a0][a1]amix=inputs=2:duration=first:normalize=0[aout]",
            "-map", "0:v", "-map", "[aout]"});
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
    }

    if (hasAudio)
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

    args.insert(args.end(), {"-flush_packets", "1", config.outputPath});

    // Log the full argument list for debugging
    {
        std::string cmdDebug = config.ffmpegPath;
        for (const auto& a : args) { cmdDebug += " [" + a + "]"; }
        SRU_LOG_DEBUG("start: FFmpeg command: " << cmdDebug);
    }

    // ── Launch FFmpeg via ProcessBridge ──
    ucf::utilities::ProcessBridgeConfig pbConfig;
    pbConfig.executablePath = config.ffmpegPath;
    pbConfig.arguments = std::move(args);
    pbConfig.pipeStdin = true;
    pbConfig.captureStdout = false;
    pbConfig.captureStderr = true;
    pbConfig.stopTimeoutMs = 10000;

    m_process = ucf::utilities::IProcessBridge::create();
    m_process->registerCallback(std::make_shared<FFmpegProcessCallback>());
    if (!m_process->start(pbConfig))
    {
        SRU_LOG_ERROR("start: ProcessBridge failed to launch FFmpeg");
        m_process.reset();
        if (hNamedPipeServer != INVALID_HANDLE_VALUE) CloseHandle(hNamedPipeServer);
        m_loopbackCapture.reset();
        return false;
    }

    m_active.store(true);

    // Start WASAPI loopback: wait for FFmpeg to connect to the named pipe,
    // then begin writing captured PCM data into it.
    // Uses overlapped ConnectNamedPipe so we can abort if FFmpeg exits early.
    if (useWasapiLoopback && hNamedPipeServer != INVALID_HANDLE_VALUE)
    {
        OVERLAPPED ov{};
        ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

        bool pipeConnected = false;
        BOOL connResult = ConnectNamedPipe(hNamedPipeServer, &ov);
        DWORD connectErr = GetLastError();

        if (connResult || connectErr == ERROR_PIPE_CONNECTED)
        {
            pipeConnected = true;
        }
        else if (connectErr == ERROR_IO_PENDING)
        {
            // Poll: wait for pipe connection OR FFmpeg exit (30s timeout)
            constexpr DWORD kPollMs = 500;
            constexpr int kMaxIter = 60;
            for (int i = 0; i < kMaxIter; ++i)
            {
                if (WaitForSingleObject(ov.hEvent, kPollMs) == WAIT_OBJECT_0)
                {
                    pipeConnected = true;
                    break;
                }
                if (!m_process || !m_process->isRunning())
                {
                    SRU_LOG_ERROR("start: FFmpeg exited before connecting to named pipe");
                    CancelIoEx(hNamedPipeServer, &ov);
                    break;
                }
            }
            if (!pipeConnected)
            {
                SRU_LOG_WARN("start: timed out waiting for FFmpeg to connect to named pipe");
                CancelIoEx(hNamedPipeServer, &ov);
            }
        }
        else
        {
            SRU_LOG_ERROR("start: ConnectNamedPipe failed, error=" << connectErr);
        }

        if (ov.hEvent) CloseHandle(ov.hEvent);

        if (!pipeConnected)
        {
            CloseHandle(hNamedPipeServer);
            m_loopbackCapture.reset();
        }
        else
        {
            if (!m_loopbackCapture->start(config.systemAudioDevice, hNamedPipeServer))
            {
                SRU_LOG_ERROR("start: WasapiLoopbackCapture failed to start");
                CloseHandle(hNamedPipeServer);
                m_loopbackCapture.reset();
            }
            else
            {
                m_hLoopbackRead = reinterpret_cast<intptr_t>(hNamedPipeServer);
            }
        }
    }

    SRU_LOG_INFO("start: started ffmpeg pid=" << m_process->processPid()
                 << " output=" << config.outputPath);
    return true;
}

// ============================================================================
// IScreenRecorder — stop
// ============================================================================

RecordingResult ScreenRecorder_Win::stop()
{
    RecordingResult result;
    result.outputPath = m_outputPath;

    if (!m_active.load())
    {
        result.errorMessage = "Not recording";
        return result;
    }

    // Send 'q' to FFmpeg's stdin for graceful shutdown (flush + trailer).
    // Must happen BEFORE closing the loopback pipe to avoid I/O error.
    if (m_process)
    {
        m_process->writeToStdin("q\n");
        m_process->closeStdin();
    }

    // Stop WASAPI loopback capture thread (stops writing PCM data)
    if (m_loopbackCapture)
    {
        m_loopbackCapture->stop();
        m_loopbackCapture.reset();
    }
    // Close the named pipe server handle (FFmpeg sees EOF on the loopback input)
    if (m_hLoopbackRead)
    {
        CloseHandle(reinterpret_cast<HANDLE>(m_hLoopbackRead));
        m_hLoopbackRead = 0;
    }

    // Wait for FFmpeg to exit naturally after receiving 'q'.
    // FFmpeg needs time to flush encoders and write container trailer.
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

// ============================================================================
// IScreenRecorder — pause / resume
// ============================================================================

bool ScreenRecorder_Win::pause()
{
    if (!m_active.load() || !m_process)
    {
        return false;
    }

    DWORD processId = static_cast<DWORD>(m_process->processPid());
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    THREADENTRY32 te{};
    te.dwSize = sizeof(te);

    bool suspended = false;
    if (Thread32First(hSnapshot, &te))
    {
        do
        {
            if (te.th32OwnerProcessID == processId)
            {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread)
                {
                    SuspendThread(hThread);
                    CloseHandle(hThread);
                    suspended = true;
                }
            }
        } while (Thread32Next(hSnapshot, &te));
    }

    CloseHandle(hSnapshot);
    return suspended;
}

bool ScreenRecorder_Win::resume()
{
    if (!m_active.load() || !m_process)
    {
        return false;
    }

    DWORD processId = static_cast<DWORD>(m_process->processPid());
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    THREADENTRY32 te{};
    te.dwSize = sizeof(te);

    bool resumed = false;
    if (Thread32First(hSnapshot, &te))
    {
        do
        {
            if (te.th32OwnerProcessID == processId)
            {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread)
                {
                    ResumeThread(hThread);
                    CloseHandle(hThread);
                    resumed = true;
                }
            }
        } while (Thread32Next(hSnapshot, &te));
    }

    CloseHandle(hSnapshot);
    return resumed;
}

// ============================================================================
// IScreenRecorder — state query
// ============================================================================

bool ScreenRecorder_Win::isActive() const
{
    return m_active.load();
}

std::string ScreenRecorder_Win::outputPath() const
{
    return m_outputPath;
}

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
