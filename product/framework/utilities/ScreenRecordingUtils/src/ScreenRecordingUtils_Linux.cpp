#include "ScreenRecordingUtils_Linux.h"

#ifdef __linux__

#include <filesystem>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>

namespace fs = std::filesystem;

namespace ucf::utilities::screenrecording {

// ============================================================================
// Static helpers
// ============================================================================

static bool fileExists(const std::string& path)
{
    struct stat st{};
    return (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode));
}

static std::string resolvePath(const std::string& path)
{
    char resolved[PATH_MAX];
    if (realpath(path.c_str(), resolved))
    {
        return std::string(resolved);
    }
    return {};
}

/// Run a command and return its combined stdout+stderr output (trimmed).
static std::string execCmd(const std::string& cmd)
{
    std::array<char, 4096> buf{};
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        return {};
    }
    while (fgets(buf.data(), static_cast<int>(buf.size()), pipe))
    {
        result += buf.data();
    }
    pclose(pipe);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' '))
    {
        result.pop_back();
    }
    return result;
}

/// Convert a video file to a different container/codec using FFmpeg.
static bool convertVideoFormat(const std::string& ffmpegPath,
                               const std::string& inputPath,
                               const std::string& outputPath)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
    {
        return false;
    }

    if (!fileExists(inputPath))
    {
        return false;
    }

    std::string ext = fs::path(outputPath).extension().string();
    std::string codecArgs;
    if (ext == ".mp4" || ext == ".mov")
    {
        codecArgs = "-c:v libx264 -preset fast -crf 22 -pix_fmt yuv420p";
    }
    else
    {
        codecArgs = "-c copy";
    }

    std::string cmd = "\"" + ffmpegPath + "\" -y -i \"" + inputPath
                      + "\" " + codecArgs + " \"" + outputPath + "\" 2>&1";
    execCmd(cmd);

    std::error_code ec;
    auto sz = fs::file_size(outputPath, ec);
    return !ec && sz > 0;
}

// ============================================================================
// Embedded Python portal recorder script
// ============================================================================

static constexpr const char* kPortalRecorderScript = R"PYTHON(#!/usr/bin/env python3
"""
Portal ScreenCast recorder.

Uses xdg-desktop-portal ScreenCast D-Bus API to acquire a PipeWire stream,
then records via gst-launch-1.0.

Usage: portal_recorder.py <output_path> <fps> [<crop_x> <crop_y> <crop_w> <crop_h>]

Protocol (stdout):
  RECORDING         – pipeline is running, recording started
  ERROR:<message>   – fatal error, script will exit
  STOPPED           – recording stopped gracefully

Stops on SIGTERM or SIGINT.
"""
import sys, os, signal, subprocess, time, re

import dbus
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib

DBusGMainLoop(set_as_default=True)

output_path = sys.argv[1]
fps = int(sys.argv[2])

# Optional region crop parameters
crop_region = None
if len(sys.argv) >= 7:
    crop_region = {
        'x': int(sys.argv[3]),
        'y': int(sys.argv[4]),
        'w': int(sys.argv[5]),
        'h': int(sys.argv[6]),
    }

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

def probe_buffer_size(node_id):
    """Capture one frame from PipeWire to detect actual buffer dimensions."""
    try:
        r = subprocess.run(
            ['gst-launch-1.0', '-v',
             'pipewiresrc', f'path={node_id}', 'num-buffers=1', '!',
             'fakesink'],
            capture_output=True, text=True, timeout=5
        )
        for line in r.stderr.splitlines():
            m = re.search(r'width=\(int\)(\d+).*?height=\(int\)(\d+)', line)
            if m:
                return (int(m.group(1)), int(m.group(2)))
    except Exception:
        pass
    return None

bus = dbus.SessionBus()

portal_obj = bus.get_object('org.freedesktop.portal.Desktop',
                            '/org/freedesktop/portal/desktop')
screencast_iface = dbus.Interface(portal_obj, 'org.freedesktop.portal.ScreenCast')

request_counter = 0
session_path = None
pw_node_id = None
stream_size = None  # (width, height) from portal
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
    global pw_node_id, stream_size
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

    # Extract stream resolution from properties
    props = streams[0][1] if len(streams[0]) > 1 else {}
    sz = props.get('size', None)
    if sz and len(sz) == 2:
        stream_size = (int(sz[0]), int(sz[1]))

    launch_gstreamer()

def launch_gstreamer():
    global gst_proc
    ext = os.path.splitext(output_path)[1].lower()

    # Build crop element if region is requested
    crop_element = ''
    if crop_region:
        # Probe actual PipeWire buffer dimensions (physical pixels)
        actual_size = probe_buffer_size(pw_node_id)
        # Fallback to portal-reported size if probe fails
        buf_w, buf_h = actual_size if actual_size else (stream_size if stream_size else (0, 0))

        if buf_w > 0 and buf_h > 0:
            # Compute scale factor: portal's size is logical, buffer may be physical (HiDPI)
            sx, sy = 1.0, 1.0
            if stream_size and stream_size[0] > 0 and stream_size[1] > 0:
                sx = buf_w / stream_size[0]
                sy = buf_h / stream_size[1]

            # Scale crop coordinates from logical to physical pixel space
            cx = int(crop_region['x'] * sx)
            cy = int(crop_region['y'] * sy)
            cw = int(crop_region['w'] * sx)
            ch = int(crop_region['h'] * sy)

            top = max(0, cy)
            left = max(0, cx)
            bottom = max(0, buf_h - cy - ch)
            right = max(0, buf_w - cx - cw)
            if top > 0 or left > 0 or bottom > 0 or right > 0:
                crop_element = f'videocrop top={top} left={left} bottom={bottom} right={right} ! '

    # Build encode pipeline based on target format
    if ext == '.webm':
        encode = 'videoconvert ! vp8enc deadline=1 cpu-used=4 min_quantizer=10 max_quantizer=50 ! webmmux'
    elif ext in ('.mp4', '.mov'):
        encode = 'videoconvert ! x264enc tune=zerolatency speed-preset=ultrafast ! mp4mux'
    else:
        encode = 'videoconvert ! vp8enc deadline=1 cpu-used=4 ! webmmux'

    pipeline = (
        f'pipewiresrc path={pw_node_id} do-timestamp=true keepalive-time=1000 ! '
        f'videorate ! video/x-raw,framerate={fps}/1 ! '
        f'{crop_element}'
        f'{encode} ! filesink location={output_path}'
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
        return {};
    }
    ofs << kPortalRecorderScript;
    ofs.close();
    chmod(scriptPath.c_str(), 0700);
    return scriptPath;
}

// ============================================================================
// FFmpeg discovery
// ============================================================================

std::string ScreenRecordingUtils_Linux::findFFmpegPath(const std::string& appDir)
{
    std::vector<std::string> candidates = {
        appDir + "/ffmpeg",
        "/usr/bin/ffmpeg",
        "/usr/local/bin/ffmpeg"
    };

    for (const auto& candidate : candidates)
    {
        std::string resolved = resolvePath(candidate);
        if (!resolved.empty() && fileExists(resolved))
        {
            return resolved;
        }
    }

    return {};
}

// ============================================================================
// Recording control — Wayland (xdg-desktop-portal ScreenCast + GStreamer)
// ============================================================================

RecordingSession ScreenRecordingUtils_Linux::startRecording(const RecordingConfig& config)
{
    RecordingSession session;
    session.outputPath = config.outputPath;
    session.isWaylandScreencast = true;
    session.ffmpegPath = config.ffmpegPath;

    if (config.outputPath.empty())
    {
        return {};
    }

    // Ensure output directory exists
    std::error_code ec;
    fs::create_directories(fs::path(config.outputPath).parent_path(), ec);

    // Portal + GStreamer records into a temp .webm, then we convert to target format.
    std::string tempWebm = fs::path(config.outputPath).parent_path()
        / ("portal_rec_" + std::to_string(getpid()) + ".webm");
    session.waylandTempPath = tempWebm;

    // Write the Python portal recorder script
    std::string scriptPath = ensureRecorderScript();
    if (scriptPath.empty())
    {
        session.isWaylandScreencast = false;
        return {};
    }

    // Create a pipe for reading the child's stdout (protocol messages)
    int pipeFds[2];
    if (pipe(pipeFds) != 0)
    {
        session.isWaylandScreencast = false;
        return {};
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        close(pipeFds[0]);
        close(pipeFds[1]);
        session.isWaylandScreencast = false;
        return {};
    }

    if (pid == 0)
    {
        // Child process
        close(pipeFds[0]); // close read end
        dup2(pipeFds[1], STDOUT_FILENO);
        close(pipeFds[1]);

        // Redirect stderr to /dev/null (Python/GStreamer noise)
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull >= 0)
        {
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }

        std::string fpsStr = std::to_string(config.fps);

        if (config.isRegion && config.regionW > 0 && config.regionH > 0)
        {
            std::string rxStr = std::to_string(config.regionX);
            std::string ryStr = std::to_string(config.regionY);
            std::string rwStr = std::to_string(config.regionW);
            std::string rhStr = std::to_string(config.regionH);
            execlp("python3", "python3", scriptPath.c_str(),
                   tempWebm.c_str(), fpsStr.c_str(),
                   rxStr.c_str(), ryStr.c_str(),
                   rwStr.c_str(), rhStr.c_str(), nullptr);
        }
        else
        {
            execlp("python3", "python3", scriptPath.c_str(),
                   tempWebm.c_str(), fpsStr.c_str(), nullptr);
        }
        _exit(127);
    }

    // Parent process
    close(pipeFds[1]); // close write end
    session.pid = pid;
    session.stdinFd = pipeFds[0]; // repurpose stdinFd as read-end of stdout pipe

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
        // Kill the child if still alive
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
        close(pipeFds[0]);
        session.pid = -1;
        session.isWaylandScreencast = false;
        return {};
    }

    return session;
}

RecordingResult ScreenRecordingUtils_Linux::stopRecording(RecordingSession& session)
{
    if (!session.isValid())
    {
        return {false, {}, "Invalid session"};
    }

    // Send SIGTERM to the Python script; it will SIGINT gst-launch → EOS → clean file
    if (session.pid > 0)
    {
        kill(static_cast<pid_t>(session.pid), SIGTERM);

        // Wait for exit (max 8 seconds — GStreamer EOS finalization can take a moment)
        constexpr int kMaxWaitMs = 8000;
        constexpr int kPollMs = 100;
        int waited = 0;
        while (waited < kMaxWaitMs)
        {
            int status = 0;
            pid_t r = waitpid(static_cast<pid_t>(session.pid), &status, WNOHANG);
            if (r != 0)
            {
                break;
            }
            usleep(kPollMs * 1000);
            waited += kPollMs;
        }

        // If still alive, force kill
        if (waited >= kMaxWaitMs)
        {
            kill(static_cast<pid_t>(session.pid), SIGKILL);
            waitpid(static_cast<pid_t>(session.pid), nullptr, 0);
        }
    }

    // Close the stdout pipe
    if (session.stdinFd >= 0)
    {
        close(session.stdinFd);
        session.stdinFd = -1;
    }

    session.pid = -1;
    session.isWaylandScreencast = false;

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
        auto sz = fs::file_size(session.waylandTempPath, ec);
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

    if (!fileExists(session.waylandTempPath))
    {
        return {false, {}, "Recording file not produced"};
    }

    // Verify the file has actual video content
    {
        std::error_code ec2;
        auto sz = fs::file_size(session.waylandTempPath, ec2);
        if (ec2 || sz < 1024)
        {
            fs::remove(session.waylandTempPath, ec2);
            return {false, {}, "Recording too short — no video frames captured"};
        }
    }

    // If the requested format is WebM, just rename the temp file
    std::string targetExt = fs::path(session.outputPath).extension().string();
    if (targetExt == ".webm")
    {
        std::error_code ec;
        fs::rename(session.waylandTempPath, session.outputPath, ec);
        if (ec)
        {
            fs::copy_file(session.waylandTempPath, session.outputPath,
                          fs::copy_options::overwrite_existing, ec);
            fs::remove(session.waylandTempPath, ec);
        }
        if (fileExists(session.outputPath))
        {
            return {true, session.outputPath, {}};
        }
        return {false, {}, "Failed to move recording file"};
    }

    // Convert webm → target format (mp4, mov, etc.)
    bool ok = convertVideoFormat(session.ffmpegPath,
                                 session.waylandTempPath,
                                 session.outputPath);
    std::error_code ec;
    fs::remove(session.waylandTempPath, ec);

    if (ok)
    {
        return {true, session.outputPath, {}};
    }
    return {false, {}, "Format conversion from WebM failed"};
}

bool ScreenRecordingUtils_Linux::pauseRecording(const RecordingSession& session)
{
    // Pause GStreamer pipeline by sending SIGSTOP to the Python process group
    if (session.pid > 0)
    {
        kill(static_cast<pid_t>(session.pid), SIGSTOP);
        return true;
    }
    return false;
}

bool ScreenRecordingUtils_Linux::resumeRecording(const RecordingSession& session)
{
    // Resume GStreamer pipeline by sending SIGCONT
    if (session.pid > 0)
    {
        kill(static_cast<pid_t>(session.pid), SIGCONT);
        return true;
    }
    return false;
}

bool ScreenRecordingUtils_Linux::convertToGif(const std::string& ffmpegPath,
                                              const std::string& inputPath,
                                              const std::string& outputPath,
                                              int fps)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
    {
        return false;
    }

    std::string filterComplex = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    std::string cmd = "\"" + ffmpegPath + "\" -y -i \"" + inputPath
                      + "\" -filter_complex \"" + filterComplex + "\" \""
                      + outputPath + "\" 2>&1";
    execCmd(cmd);

    std::error_code ec;
    auto sz = fs::file_size(outputPath, ec);
    return !ec && sz > 0;
}

} // namespace ucf::utilities::screenrecording

#endif // __linux__
