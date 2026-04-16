#include "ScreenRecorder_Win.h"

#ifdef _WIN32

#include "LoggerDefine.h"
#include "WasapiLoopbackCapture.h"

#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
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

/// Wait for a process with timeout, then force-terminate if still running.
static bool waitForProcessWithTimeout(HANDLE hProcess, DWORD timeoutMs, std::string& errorMsg)
{
    DWORD waitResult = WaitForSingleObject(hProcess, timeoutMs);
    if (waitResult == WAIT_TIMEOUT)
    {
        SRU_LOG_WARN("Process did not exit in " << timeoutMs << "ms, force terminating");
        TerminateProcess(hProcess, 1);
        WaitForSingleObject(hProcess, 5000);
        errorMsg = "FFmpeg did not exit in time, force killed";
        return false;
    }
    return true;
}

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

    // Create a pipe for FFmpeg's stdin so we can send 'q' to stop it.
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE hStdinRead  = nullptr;
    HANDLE hStdinWrite = nullptr;
    if (!CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0))
    {
        SRU_LOG_ERROR("start: CreatePipe (stdin) failed, error=" << GetLastError());
        return false;
    }
    SetHandleInformation(hStdinWrite, HANDLE_FLAG_INHERIT, 0);

    // ── WASAPI loopback pipe (if system audio via OutputDevice) ──
    bool useWasapiLoopback = false;
    std::string namedPipePath;
    HANDLE hNamedPipeServer = INVALID_HANDLE_VALUE;

    bool needSystemAudio = (config.audioMode == AudioCaptureMode::SystemAudio ||
                            config.audioMode == AudioCaptureMode::MicAndSystem);

    if (needSystemAudio && config.systemAudioDeviceType == AudioDeviceType::OutputDevice)
    {
        // Create a named pipe for WASAPI loopback → FFmpeg.
        // FFmpeg on Windows can open named pipes as file paths (unlike pipe:N FDs).
        namedPipePath = "\\\\.\\pipe\\sru_loopback_" + std::to_string(GetCurrentProcessId())
                      + "_" + std::to_string(GetTickCount64());

        hNamedPipeServer = CreateNamedPipeA(
            namedPipePath.c_str(),
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1,       // max instances
            65536,   // out buffer size
            0,       // in buffer size (not used for outbound)
            0,       // default timeout
            nullptr);

        if (hNamedPipeServer == INVALID_HANDLE_VALUE)
        {
            SRU_LOG_ERROR("start: CreateNamedPipe failed, error=" << GetLastError());
            CloseHandle(hStdinRead);
            CloseHandle(hStdinWrite);
            return false;
        }

        // Probe the device format BEFORE starting capture, so we can put
        // -ar/-ac on the FFmpeg command line.
        m_loopbackCapture = std::make_unique<WasapiLoopbackCapture>();
        if (!m_loopbackCapture->probeFormat(config.systemAudioDevice))
        {
            SRU_LOG_ERROR("start: WasapiLoopbackCapture failed to probe device format");
            CloseHandle(hNamedPipeServer);
            CloseHandle(hStdinRead);
            CloseHandle(hStdinWrite);
            m_loopbackCapture.reset();
            return false;
        }

        useWasapiLoopback = true;
    }

    // ── Build FFmpeg command line ──
    std::ostringstream cmdLine;
    cmdLine << "\"" << config.ffmpegPath << "\""
            << " -y"
            << " -thread_queue_size 512"
            << " -probesize 5M"
            << " -f gdigrab"
            << " -framerate " << config.fps
            << " -draw_mouse 1";

    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        int w = alignToEven(config.regionW);
        int h = alignToEven(config.regionH);
        cmdLine << " -offset_x " << config.regionX
                << " -offset_y " << config.regionY
                << " -video_size " << w << "x" << h;
    }

    cmdLine << " -i desktop";

    // ── Audio inputs ──
    bool hasAudio = (config.audioMode != AudioCaptureMode::None);
    bool needAudioMix = false;

    // Microphone input (always via DirectShow)
    if (config.audioMode == AudioCaptureMode::Microphone ||
        config.audioMode == AudioCaptureMode::MicAndSystem)
    {
        std::string micName = config.micDevice.empty()
            ? "Microphone" : escapeDshowDeviceName(config.micDevice);
        cmdLine << " -thread_queue_size 512 -f dshow -i audio=\"" << micName << "\"";
    }

    // System audio input
    if (needSystemAudio)
    {
        if (useWasapiLoopback)
        {
            // WASAPI loopback PCM data comes from a named pipe.
            // -use_wallclock_as_timestamps 1 is critical: without it, FFmpeg
            // assigns byte-offset-based timestamps starting at 0 for the raw
            // s16le stream. The other inputs (gdigrab, dshow) use wall-clock
            // timestamps. The amix filter would try to sync them, waiting
            // forever for the pipe stream to "catch up" → deadlock, 0 frames.
            cmdLine << " -use_wallclock_as_timestamps 1"
                    << " -thread_queue_size 512"
                    << " -f s16le"
                    << " -ar " << m_loopbackCapture->sampleRate()
                    << " -ac " << m_loopbackCapture->channels()
                    << " -i \"" << namedPipePath << "\"";
        }
        else
        {
            // Traditional DirectShow loopback (Stereo Mix etc.)
            std::string sysName = config.systemAudioDevice.empty()
                ? "Stereo Mix" : escapeDshowDeviceName(config.systemAudioDevice);
            cmdLine << " -thread_queue_size 512 -f dshow -i audio=\"" << sysName << "\"";
        }

        if (config.audioMode == AudioCaptureMode::MicAndSystem)
        {
            needAudioMix = true;
        }
    }

    // Audio mixing filter for MicAndSystem mode.
    // asetpts=PTS-STARTPTS resets each audio stream's timestamps to start at 0.
    // Without this, dshow uses the DirectShow reference clock (~system uptime)
    // while the WASAPI named pipe uses wall-clock timestamps (Unix epoch).
    // The ~56-year gap causes amix to deadlock waiting to align them.
    if (needAudioMix)
    {
        cmdLine << " -filter_complex \""
                   "[1:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo,asetpts=PTS-STARTPTS[a0];"
                   "[2:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo,asetpts=PTS-STARTPTS[a1];"
                   "[a0][a1]amix=inputs=2:duration=first:normalize=0[aout]\""
                   " -map 0:v -map \"[aout]\"";
    }

    // Output codec settings
    if (config.videoFormat == "webm")
    {
        cmdLine << " -c:v libvpx-vp9 -b:v 2M";
    }
    else
    {
        cmdLine << " -c:v libx264 -preset ultrafast -tune zerolatency -pix_fmt yuv420p";
    }

    if (hasAudio)
    {
        cmdLine << " -ar 48000 -ac 2";
        if (config.videoFormat == "webm")
        {
            cmdLine << " -c:a libopus -b:a 128k";
        }
        else
        {
            cmdLine << " -c:a aac -b:a 128k";
        }
    }

    cmdLine << " -flush_packets 1";
    cmdLine << " \"" << config.outputPath << "\"";

    std::string cmdStr = cmdLine.str();
    std::vector<char> cmdBuf(cmdStr.begin(), cmdStr.end());
    cmdBuf.push_back('\0');

    // Redirect FFmpeg's stderr to a log file
    std::string logPath = config.outputPath + ".ffmpeg.log";
    HANDLE hLogFile = CreateFileA(
        logPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &sa,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput  = hStdinRead;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError  = (hLogFile != INVALID_HANDLE_VALUE) ? hLogFile
                                                        : GetStdHandle(STD_ERROR_HANDLE);
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessA(
        nullptr, cmdBuf.data(), nullptr, nullptr,
        TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);

    CloseHandle(hStdinRead);
    if (hLogFile != INVALID_HANDLE_VALUE) CloseHandle(hLogFile);

    if (!ok)
    {
        SRU_LOG_ERROR("start: CreateProcess failed, error=" << GetLastError());
        CloseHandle(hStdinWrite);
        if (hNamedPipeServer != INVALID_HANDLE_VALUE) CloseHandle(hNamedPipeServer);
        m_loopbackCapture.reset();
        return false;
    }

    CloseHandle(pi.hThread);

    m_hProcess = reinterpret_cast<intptr_t>(pi.hProcess);
    m_hStdinWrite = reinterpret_cast<intptr_t>(hStdinWrite);
    m_active.store(true);

    // Start WASAPI loopback: wait for FFmpeg to connect to the named pipe,
    // then begin writing captured PCM data into it.
    if (useWasapiLoopback && hNamedPipeServer != INVALID_HANDLE_VALUE)
    {
        // ConnectNamedPipe blocks until FFmpeg opens the pipe path during
        // its input probing. FFmpeg probes inputs sequentially (gdigrab,
        // dshow mic, then named pipe), so this may block for a few seconds.
        BOOL connected = ConnectNamedPipe(hNamedPipeServer, nullptr);
        DWORD connectErr = GetLastError();

        if (!connected && connectErr != ERROR_PIPE_CONNECTED)
        {
            SRU_LOG_ERROR("start: FFmpeg did not connect to named pipe, error=" << connectErr);
            CloseHandle(hNamedPipeServer);
            m_loopbackCapture.reset();
        }
        else
        {
            // Start the WASAPI capture thread, writing PCM to the pipe
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

    SRU_LOG_INFO("start: started ffmpeg pid=" << pi.dwProcessId
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

    HANDLE hProcess = reinterpret_cast<HANDLE>(m_hProcess);
    HANDLE hStdinWrite = reinterpret_cast<HANDLE>(m_hStdinWrite);

    // Send 'q\n' to FFmpeg's stdin FIRST — this triggers a graceful shutdown
    // where FFmpeg flushes its encoders and writes the container trailer.
    // We must send 'q' BEFORE closing the loopback pipe, otherwise FFmpeg
    // may abort with an I/O error on the pipe input instead of flushing.
    if (hStdinWrite)
    {
        const char quitCmd[] = "q\n";
        DWORD written = 0;
        if (!WriteFile(hStdinWrite, quitCmd, sizeof(quitCmd) - 1, &written, nullptr))
        {
            SRU_LOG_WARN("stop: WriteFile to stdin pipe failed, error=" << GetLastError());
        }
        CloseHandle(hStdinWrite);
        m_hStdinWrite = 0;
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

    // Wait for process to exit (up to 10 seconds)
    std::string killError;
    waitForProcessWithTimeout(hProcess, 10000, killError);
    if (!killError.empty())
    {
        result.errorMessage = killError;
    }

    CloseHandle(hProcess);
    m_hProcess = 0;
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
    if (!m_active.load())
    {
        return false;
    }

    HANDLE hProcess = reinterpret_cast<HANDLE>(m_hProcess);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD processId = GetProcessId(hProcess);
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
    if (!m_active.load())
    {
        return false;
    }

    HANDLE hProcess = reinterpret_cast<HANDLE>(m_hProcess);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD processId = GetProcessId(hProcess);
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
