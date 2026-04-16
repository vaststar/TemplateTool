#include "ScreenRecorder_Linux.h"

#ifdef __linux__

#include "LoggerDefine.h"

#include <cerrno>
#include <cstring>
#include <climits>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

namespace ucf::utilities::screenrecording {

// ============================================================================
// Static helpers
// ============================================================================

/// Round a dimension down to the nearest even number (minimum 2).
/// libx264 + yuv420p requires even width and height.
static inline int alignToEven(int v)
{
    return std::max(2, v & ~1);
}

/// Set close-on-exec flag on a file descriptor.
static inline void setCloseOnExec(int fd)
{
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
}

/// Wait for a child process with timeout, escalating from SIGINT to SIGKILL.
/// Returns true if process exited normally, false if force-killed.
static bool waitForChildWithTimeout(pid_t pid, int& status, int timeoutMs, std::string& errorMsg)
{
    constexpr int kPollMs = 100;

    // Phase 1: wait for voluntary exit
    int elapsed = 0;
    while (elapsed < timeoutMs)
    {
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r != 0)
        {
            return true;
        }
        usleep(kPollMs * 1000);
        elapsed += kPollMs;
    }

    // Phase 2: SIGINT for graceful finalization (5 seconds)
    SRU_LOG_WARN("Child process " << pid << " did not exit in " << timeoutMs << "ms, sending SIGINT");
    kill(pid, SIGINT);

    constexpr int kIntTimeoutMs = 5000;
    elapsed = 0;
    while (elapsed < kIntTimeoutMs)
    {
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r != 0)
        {
            return true;
        }
        usleep(kPollMs * 1000);
        elapsed += kPollMs;
    }

    // Phase 3: SIGKILL as last resort
    SRU_LOG_ERROR("Child process " << pid << " did not respond to SIGINT, sending SIGKILL");
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    errorMsg = "Process did not exit in time, force killed";
    return false;
}

/// Spawn FFmpeg (or any executable) with args, wait with timeout.
/// stdout/stderr are redirected to /dev/null.
/// Returns true if the process exited with code 0.
static bool spawnAndWait(const std::string& executable,
                         const std::vector<std::string>& args,
                         int timeoutMs)
{
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (const auto& a : args)
    {
        argv.push_back(const_cast<char*>(a.c_str()));
    }
    argv.push_back(nullptr);

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);

    pid_t pid = -1;
    int rc = posix_spawn(&pid, executable.c_str(), &actions, nullptr,
                         argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);

    if (rc != 0)
    {
        SRU_LOG_ERROR("posix_spawn failed for " << executable << ": " << std::strerror(rc));
        return false;
    }

    int status = 0;
    std::string killMsg;
    waitForChildWithTimeout(pid, status, timeoutMs, killMsg);

    if (!killMsg.empty())
    {
        SRU_LOG_WARN(killMsg);
        return false;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/// Convert a video file to a different container/codec using FFmpeg.
/// Optionally crops to region (x, y, w, h) if cropW > 0.
static bool convertVideoFormat(const std::string& ffmpegPath,
                               const std::string& inputPath,
                               const std::string& outputPath,
                               int cropX = 0, int cropY = 0,
                               int cropW = 0, int cropH = 0)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
    {
        SRU_LOG_ERROR("convertVideoFormat: empty path argument");
        return false;
    }

    std::error_code ec;
    if (!std::filesystem::is_regular_file(inputPath, ec))
    {
        SRU_LOG_ERROR("convertVideoFormat: input file does not exist: " << inputPath);
        return false;
    }

    std::string ext = std::filesystem::path(outputPath).extension().string();

    // Build FFmpeg argv
    std::vector<std::string> args = {ffmpegPath, "-y", "-i", inputPath};

    int cw = alignToEven(cropW);
    int ch = alignToEven(cropH);

    if (ext == ".mp4" || ext == ".mov")
    {
        if (cropW > 0 && cropH > 0)
        {
            std::string cropFilter = "crop=" + std::to_string(cw) + ":"
                                   + std::to_string(ch) + ":" + std::to_string(cropX)
                                   + ":" + std::to_string(cropY) + ",format=yuv420p";
            args.insert(args.end(), {"-vf", cropFilter});
        }
        else
        {
            args.insert(args.end(), {"-pix_fmt", "yuv420p"});
        }
        args.insert(args.end(), {"-c:v", "libx264", "-preset", "fast", "-crf", "22"});
    }
    else
    {
        if (cropW > 0 && cropH > 0)
        {
            std::string cropFilter = "crop=" + std::to_string(cw) + ":" + std::to_string(ch)
                                   + ":" + std::to_string(cropX) + ":" + std::to_string(cropY);
            args.insert(args.end(), {"-vf", cropFilter, "-c:v", "libvpx", "-b:v", "2M"});
        }
        else
        {
            args.insert(args.end(), {"-c", "copy"});
        }
    }

    args.push_back(outputPath);

    SRU_LOG_INFO("convertVideoFormat: " << inputPath << " -> " << outputPath);
    constexpr int kConvertTimeoutMs = 120000;
    bool ok = spawnAndWait(ffmpegPath, args, kConvertTimeoutMs);

    auto sz = std::filesystem::file_size(outputPath, ec);
    return ok && !ec && sz > 0;
}

// ============================================================================
// Embedded Python portal recorder script
// ============================================================================

static constexpr const char* kPortalRecorderScript = R"PYTHON(#!/usr/bin/env python3
"""
Portal ScreenCast recorder.

Uses xdg-desktop-portal ScreenCast D-Bus API to acquire a PipeWire stream,
then records via gst-launch-1.0.

Usage: portal_recorder.py <output_path> <fps> <audio_mode> [<mic_device>] [<sys_device>]
  audio_mode: none / microphone / system / mic_and_system

Protocol (stdout):
  RECORDING         – pipeline is running, recording started
  ERROR:<message>   – fatal error, script will exit
  STOPPED           – recording stopped gracefully

Stops on SIGTERM or SIGINT.
"""
import sys, os, signal, subprocess, time

import dbus
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib

DBusGMainLoop(set_as_default=True)

output_path = sys.argv[1]
fps = int(sys.argv[2])
audio_mode = sys.argv[3] if len(sys.argv) > 3 else 'none'
mic_device = sys.argv[4] if len(sys.argv) > 4 else ''
sys_device = sys.argv[5] if len(sys.argv) > 5 else ''

# Token persistence file (persist_mode=2 lets portal skip the dialog)
TOKEN_FILE = os.path.expanduser('~/.cache/portal_screencast_token')

def load_restore_token():
    try:
        with open(TOKEN_FILE, 'r') as f:
            return f.read().strip()
    except Exception:
        return ''

def save_restore_token(token):
    try:
        os.makedirs(os.path.dirname(TOKEN_FILE), exist_ok=True)
        with open(TOKEN_FILE, 'w') as f:
            f.write(token)
    except Exception:
        pass

bus = dbus.SessionBus()

portal_obj = bus.get_object('org.freedesktop.portal.Desktop',
                            '/org/freedesktop/portal/desktop')
screencast_iface = dbus.Interface(portal_obj, 'org.freedesktop.portal.ScreenCast')

request_counter = 0
session_path = None
pw_node_id = None
gst_proc = None
loop = GLib.MainLoop()

def new_request_token():
    global request_counter
    request_counter += 1
    return f'u{os.getpid()}_{request_counter}'

def new_session_token():
    return f'u{os.getpid()}_session'

def emit_error(msg):
    print(f'ERROR:{msg}', flush=True)
    loop.quit()

def on_start_response(response, results):
    global pw_node_id
    if response != 0:
        emit_error(f'Start rejected (response={response})')
        return

    # Save restore_token for next run (skip dialog next time)
    restore_token = str(results.get('restore_token', ''))
    if restore_token:
        save_restore_token(restore_token)

    streams = results.get('streams', [])
    if not streams:
        emit_error('No streams returned by portal')
        return
    pw_node_id = int(streams[0][0])
    launch_gstreamer()

def launch_gstreamer():
    global gst_proc
    ext = os.path.splitext(output_path)[1].lower()

    # Always record full screen; region cropping is done via FFmpeg after recording.
    # Build encode pipeline based on target format
    if ext == '.webm':
        vencode = 'videoconvert ! vp8enc deadline=1 cpu-used=4 min_quantizer=10 max_quantizer=50'
        mux = 'webmmux'
        aencode = 'audioconvert ! opusenc'
    elif ext in ('.mp4', '.mov'):
        vencode = 'videoconvert ! x264enc tune=zerolatency speed-preset=ultrafast'
        mux = 'mp4mux'
        aencode = 'audioconvert ! avenc_aac'
    else:
        vencode = 'videoconvert ! vp8enc deadline=1 cpu-used=4'
        mux = 'webmmux'
        aencode = 'audioconvert ! opusenc'

    # Build audio source elements based on audio_mode
    audio_pipeline = ''
    if audio_mode == 'microphone':
        src = f'pulsesrc device={mic_device}' if mic_device else 'pulsesrc'
        audio_pipeline = f'{src} ! {aencode} ! queue ! mux.'
    elif audio_mode == 'system':
        src = f'pulsesrc device={sys_device}' if sys_device else 'pulsesrc device=auto_default.monitor'
        audio_pipeline = f'{src} ! {aencode} ! queue ! mux.'
    elif audio_mode == 'mic_and_system':
        mic_src = f'pulsesrc device={mic_device}' if mic_device else 'pulsesrc'
        sys_src = f'pulsesrc device={sys_device}' if sys_device else 'pulsesrc device=auto_default.monitor'
        audio_pipeline = (
            f'{mic_src} ! audioconvert ! audioresample ! queue ! mix. '
            f'{sys_src} ! audioconvert ! audioresample ! queue ! mix. '
            f'audiomixer name=mix ! {aencode} ! queue ! mux.'
        )

    pipeline = (
        f'pipewiresrc path={pw_node_id} do-timestamp=true keepalive-time=1000 ! '
        f'videorate ! video/x-raw,framerate={fps}/1 ! '
        f'{vencode} ! queue ! mux. '
        f'{mux} name=mux ! filesink location={output_path} '
        f'{audio_pipeline}'
    )

    try:
        gst_proc = subprocess.Popen(
            ['gst-launch-1.0', '-e'] + pipeline.split(),
            stdin=subprocess.DEVNULL,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
    except Exception as e:
        emit_error(f'Failed to launch gst-launch-1.0: {e}')
        return

    # Give GStreamer a moment to start, verify it's still alive
    time.sleep(0.3)
    if gst_proc.poll() is not None:
        emit_error(f'gst-launch-1.0 exited immediately (rc={gst_proc.returncode})')
        return

    print('RECORDING', flush=True)

def on_select_sources_response(response, results):
    if response != 0:
        emit_error(f'SelectSources rejected (response={response})')
        return
    # Step 3: Start — this triggers the system permission dialog (or auto-grants with token)
    token = new_request_token()
    req_path = f'/org/freedesktop/portal/desktop/request/{bus.get_unique_name()[1:].replace(".",  "_")}/{token}'
    bus.add_signal_receiver(
        on_start_response,
        signal_name='Response',
        dbus_interface='org.freedesktop.portal.Request',
        path=req_path,
    )
    screencast_iface.Start(
        session_path, '',
        dbus.Dictionary({'handle_token': token}, signature='sv'),
    )

def on_create_session_response(response, results):
    global session_path
    if response != 0:
        emit_error(f'CreateSession rejected (response={response})')
        return
    session_path = str(results.get('session_handle', ''))
    if not session_path:
        emit_error('No session_handle returned')
        return
    # Step 2: SelectSources with persist_mode + restore_token
    token = new_request_token()
    req_path = f'/org/freedesktop/portal/desktop/request/{bus.get_unique_name()[1:].replace(".", "_")}/{token}'
    bus.add_signal_receiver(
        on_select_sources_response,
        signal_name='Response',
        dbus_interface='org.freedesktop.portal.Request',
        path=req_path,
    )
    select_opts = {
        'handle_token': token,
        'types': dbus.UInt32(1),       # MONITOR
        'cursor_mode': dbus.UInt32(2),  # EMBEDDED in stream
        'persist_mode': dbus.UInt32(2), # Persist until explicitly revoked
    }
    # Restore previous session token to skip permission dialog
    saved_token = load_restore_token()
    if saved_token:
        select_opts['restore_token'] = saved_token

    screencast_iface.SelectSources(
        session_path,
        dbus.Dictionary(select_opts, signature='sv'),
    )

def stop_recording(signum=None, frame=None):
    if gst_proc and gst_proc.poll() is None:
        gst_proc.send_signal(signal.SIGINT)  # -e flag → EOS → clean close
        try:
            gst_proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            gst_proc.kill()
            gst_proc.wait()
    print('STOPPED', flush=True)
    loop.quit()

signal.signal(signal.SIGTERM, stop_recording)
signal.signal(signal.SIGINT, stop_recording)

# Step 1: CreateSession
token = new_request_token()
session_token = new_session_token()
req_path = f'/org/freedesktop/portal/desktop/request/{bus.get_unique_name()[1:].replace(".", "_")}/{token}'
bus.add_signal_receiver(
    on_create_session_response,
    signal_name='Response',
    dbus_interface='org.freedesktop.portal.Request',
    path=req_path,
)
screencast_iface.CreateSession(
    dbus.Dictionary({
        'handle_token': token,
        'session_handle_token': session_token,
    }, signature='sv'),
)

loop.run()
)PYTHON";

/// Write the portal recorder script to a temp file and return its path.
static std::string ensureRecorderScript()
{
    std::string scriptPath = "/tmp/portal_recorder_" + std::to_string(getuid()) + ".py";

    // Always overwrite to ensure latest version
    std::ofstream ofs(scriptPath, std::ios::trunc);
    if (!ofs.is_open())
    {
        SRU_LOG_ERROR("ensureRecorderScript: failed to write " << scriptPath);
        return {};
    }
    ofs << kPortalRecorderScript;
    ofs.close();
    chmod(scriptPath.c_str(), 0700);
    return scriptPath;
}

// ============================================================================
// Constructor / Destructor / Queries
// ============================================================================

ScreenRecorder_Linux::ScreenRecorder_Linux() = default;

ScreenRecorder_Linux::~ScreenRecorder_Linux()
{
    if (m_active)
    {
        stop();
    }
}

bool ScreenRecorder_Linux::isActive() const
{
    return m_active;
}

std::string ScreenRecorder_Linux::outputPath() const
{
    return m_outputPath;
}

// ============================================================================
// Recording control — Wayland (xdg-desktop-portal ScreenCast + GStreamer)
// ============================================================================

bool ScreenRecorder_Linux::start(const RecordingConfig& config)
{
    if (m_active)
    {
        SRU_LOG_WARN("start: already recording");
        return false;
    }

    m_outputPath = config.outputPath;
    m_ffmpegPath = config.ffmpegPath;

    if (config.outputPath.empty())
    {
        SRU_LOG_ERROR("start: outputPath is empty");
        return false;
    }

    // Ensure output directory exists
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(config.outputPath).parent_path(), ec);

    // Portal + GStreamer records into a temp .webm, then we convert to target format.
    std::string tempWebm = std::filesystem::path(config.outputPath).parent_path()
        / ("portal_rec_" + std::to_string(getpid()) + ".webm");
    m_waylandTempPath = tempWebm;

    // Write the Python portal recorder script
    std::string scriptPath = ensureRecorderScript();
    if (scriptPath.empty())
    {
        SRU_LOG_ERROR("start: failed to write portal recorder script");
        return false;
    }

    // Create a pipe for reading the child's stdout (protocol messages)
    int pipeFds[2];
    if (pipe(pipeFds) != 0)
    {
        SRU_LOG_ERROR("start: pipe() failed: " << std::strerror(errno));
        return false;
    }
    setCloseOnExec(pipeFds[0]);
    setCloseOnExec(pipeFds[1]);

    // Find python3 on PATH
    std::string python3Path = findInPath("python3");
    if (python3Path.empty())
    {
        python3Path = "/usr/bin/python3";
    }

    std::string fpsStr = std::to_string(config.fps);

    // Map audio mode to string for the Python script
    std::string audioModeStr = "none";
    switch (config.audioMode)
    {
    case AudioCaptureMode::None:         audioModeStr = "none"; break;
    case AudioCaptureMode::Microphone:   audioModeStr = "microphone"; break;
    case AudioCaptureMode::SystemAudio:  audioModeStr = "system"; break;
    case AudioCaptureMode::MicAndSystem: audioModeStr = "mic_and_system"; break;
    }

    // posix_spawn file actions: redirect stdout to pipe, stderr to /dev/null
    posix_spawn_file_actions_t fileActions;
    posix_spawn_file_actions_init(&fileActions);
    posix_spawn_file_actions_adddup2(&fileActions, pipeFds[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&fileActions, pipeFds[0]);
    posix_spawn_file_actions_addclose(&fileActions, pipeFds[1]);
    posix_spawn_file_actions_addopen(&fileActions, STDERR_FILENO,
                                     "/dev/null", O_WRONLY, 0);

    std::vector<char*> argv = {
        const_cast<char*>(python3Path.c_str()),
        const_cast<char*>(scriptPath.c_str()),
        const_cast<char*>(tempWebm.c_str()),
        const_cast<char*>(fpsStr.c_str()),
        const_cast<char*>(audioModeStr.c_str()),
        const_cast<char*>(config.micDevice.c_str()),
        const_cast<char*>(config.systemAudioDevice.c_str()),
        nullptr
    };

    pid_t pid = -1;
    int spawnRc = posix_spawn(&pid, python3Path.c_str(),
                              &fileActions, nullptr,
                              argv.data(), environ);
    posix_spawn_file_actions_destroy(&fileActions);

    // Close write end in parent (read end kept for protocol messages)
    close(pipeFds[1]);

    if (spawnRc != 0)
    {
        SRU_LOG_ERROR("start: posix_spawn failed: " << std::strerror(spawnRc));
        close(pipeFds[0]);
        return false;
    }

    m_pid = pid;
    m_stdoutPipeFd = pipeFds[0];

    // Store region info for post-recording FFmpeg crop
    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        m_isRegion = true;
        m_regionX = config.regionX;
        m_regionY = config.regionY;
        m_regionW = config.regionW;
        m_regionH = config.regionH;
    }

    // Wait for "RECORDING" or "ERROR:" from the child (with timeout)
    // The portal will show a permission dialog on first use, so give generous timeout.
    constexpr int kStartTimeoutMs = 30000;

    struct pollfd pfd{};
    pfd.fd = pipeFds[0];
    pfd.events = POLLIN;

    std::string accumulated;
    bool started = false;
    int elapsed = 0;
    constexpr int kPollChunkMs = 200;

    while (elapsed < kStartTimeoutMs)
    {
        int ret = poll(&pfd, 1, kPollChunkMs);
        elapsed += kPollChunkMs;

        if (ret > 0 && (pfd.revents & POLLIN))
        {
            char buf[256];
            ssize_t n = read(pipeFds[0], buf, sizeof(buf) - 1);
            if (n > 0)
            {
                buf[n] = '\0';
                accumulated += buf;
            }
            else if (n == 0)
            {
                // Pipe closed — child exited
                break;
            }
        }
        else if (ret > 0 && (pfd.revents & (POLLHUP | POLLERR)))
        {
            break;
        }

        // Check for protocol messages
        if (accumulated.find("RECORDING") != std::string::npos)
        {
            started = true;
            break;
        }
        if (accumulated.find("ERROR:") != std::string::npos)
        {
            SRU_LOG_ERROR("start: portal script error: " << accumulated);
            break;
        }

        // Check if child is still alive
        int status = 0;
        if (waitpid(pid, &status, WNOHANG) != 0)
        {
            break;
        }
    }

    if (!started)
    {
        SRU_LOG_ERROR("start: portal recorder did not start within timeout");
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
        close(pipeFds[0]);
        m_pid = -1;
        m_stdoutPipeFd = -1;
        return false;
    }

    m_active = true;
    SRU_LOG_INFO("start: portal recorder started, pid=" << pid
                 << " output=" << config.outputPath);
    return true;
}

RecordingResult ScreenRecorder_Linux::stop()
{
    RecordingResult result;
    result.outputPath = m_outputPath;

    if (!m_active || m_pid <= 0)
    {
        result.errorMessage = "No active recording";
        SRU_LOG_ERROR("stop: " << result.errorMessage);
        return result;
    }

    // Send SIGTERM to the Python script; it will SIGINT gst-launch → EOS → clean file
    SRU_LOG_INFO("stop: sending SIGTERM to pid " << m_pid);
    kill(static_cast<pid_t>(m_pid), SIGTERM);

    // Wait for exit (max 8 seconds — GStreamer EOS finalization can take a moment)
    int status = 0;
    std::string killMsg;
    waitForChildWithTimeout(static_cast<pid_t>(m_pid), status, 8000, killMsg);

    if (!killMsg.empty())
    {
        SRU_LOG_WARN("stop: " << killMsg);
    }

    // Close the stdout pipe
    if (m_stdoutPipeFd >= 0)
    {
        close(m_stdoutPipeFd);
        m_stdoutPipeFd = -1;
    }

    m_pid = -1;
    m_active = false;

    // Wait for file to stabilize (GStreamer may still be flushing)
    constexpr int kMaxStabilizeMs = 3000;
    constexpr int kPollIntervalMs = 200;
    int waited = 0;
    std::uintmax_t lastSize = 0;
    int stableCount = 0;
    while (waited < kMaxStabilizeMs)
    {
        usleep(kPollIntervalMs * 1000);
        waited += kPollIntervalMs;

        std::error_code ec;
        auto sz = std::filesystem::file_size(m_waylandTempPath, ec);
        if (ec)
        {
            continue;
        }
        if (sz > 0 && sz == lastSize)
        {
            ++stableCount;
            if (stableCount >= 2)
            {
                break;
            }
        }
        else
        {
            stableCount = 0;
        }
        lastSize = sz;
    }

    std::error_code ec;
    if (!std::filesystem::is_regular_file(m_waylandTempPath, ec))
    {
        result.errorMessage = "Recording file not produced";
        SRU_LOG_ERROR("stop: " << result.errorMessage);
        return result;
    }

    // Verify the file has actual video content
    {
        std::error_code ec2;
        auto sz = std::filesystem::file_size(m_waylandTempPath, ec2);
        if (ec2 || sz < 1024)
        {
            std::filesystem::remove(m_waylandTempPath, ec2);
            result.errorMessage = "Recording too short — no video frames captured";
            SRU_LOG_ERROR("stop: " << result.errorMessage);
            return result;
        }
    }

    // If the requested format is WebM and no region crop, just rename the temp file
    std::string targetExt = std::filesystem::path(m_outputPath).extension().string();
    if (targetExt == ".webm" && !m_isRegion)
    {
        std::filesystem::rename(m_waylandTempPath, m_outputPath, ec);
        if (ec)
        {
            std::filesystem::copy_file(m_waylandTempPath, m_outputPath,
                          std::filesystem::copy_options::overwrite_existing, ec);
            std::filesystem::remove(m_waylandTempPath, ec);
        }
        if (std::filesystem::is_regular_file(m_outputPath, ec))
        {
            result.success = true;
            SRU_LOG_INFO("stop: recording saved to " << m_outputPath);
            return result;
        }
        result.errorMessage = "Failed to move recording file";
        SRU_LOG_ERROR("stop: " << result.errorMessage);
        return result;
    }

    // Convert webm → target format (and apply region crop if needed)
    int cx = 0, cy = 0, cw = 0, ch = 0;
    if (m_isRegion && m_regionW > 0 && m_regionH > 0)
    {
        cx = m_regionX;
        cy = m_regionY;
        cw = m_regionW;
        ch = m_regionH;
    }

    bool ok = convertVideoFormat(m_ffmpegPath,
                                 m_waylandTempPath,
                                 m_outputPath,
                                 cx, cy, cw, ch);
    std::filesystem::remove(m_waylandTempPath, ec);

    if (ok)
    {
        result.success = true;
        SRU_LOG_INFO("stop: recording converted and saved to " << m_outputPath);
        return result;
    }
    result.errorMessage = "Format conversion from WebM failed";
    SRU_LOG_ERROR("stop: " << result.errorMessage);
    return result;
}

bool ScreenRecorder_Linux::pause()
{
    // Pause GStreamer pipeline by sending SIGSTOP to the Python process group
    if (m_pid > 0)
    {
        SRU_LOG_INFO("pause: sending SIGSTOP to pid " << m_pid);
        kill(static_cast<pid_t>(m_pid), SIGSTOP);
        return true;
    }
    SRU_LOG_WARN("pause: invalid pid");
    return false;
}

bool ScreenRecorder_Linux::resume()
{
    // Resume GStreamer pipeline by sending SIGCONT
    if (m_pid > 0)
    {
        SRU_LOG_INFO("resume: sending SIGCONT to pid " << m_pid);
        kill(static_cast<pid_t>(m_pid), SIGCONT);
        return true;
    }
    SRU_LOG_WARN("resume: invalid pid");
    return false;
}

} // namespace ucf::utilities::screenrecording

#endif // __linux__
