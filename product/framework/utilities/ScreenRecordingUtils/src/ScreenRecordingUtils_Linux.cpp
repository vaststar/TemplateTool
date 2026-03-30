#include "ScreenRecordingUtils_Linux.h"

#ifdef __linux__

#include "LoggerDefine.h"

#include <cerrno>
#include <cstdlib>
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

/// Search the PATH environment variable for an executable by name.
static std::string findInPath(const std::string& name)
{
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv)
    {
        return {};
    }

    std::istringstream stream(pathEnv);
    std::string dir;
    while (std::getline(stream, dir, ':'))
    {
        if (dir.empty())
        {
            continue;
        }
        auto candidate = std::filesystem::path(dir) / name;
        std::error_code ec;
        if (std::filesystem::is_regular_file(candidate, ec) && access(candidate.c_str(), X_OK) == 0)
        {
            auto canonical = std::filesystem::canonical(candidate, ec);
            if (!ec)
            {
                return canonical.string();
            }
        }
    }
    return {};
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

Usage: portal_recorder.py <output_path> <fps> [<crop_x> <crop_y> <crop_w> <crop_h>]

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
        encode = 'videoconvert ! vp8enc deadline=1 cpu-used=4 min_quantizer=10 max_quantizer=50 ! webmmux'
    elif ext in ('.mp4', '.mov'):
        encode = 'videoconvert ! x264enc tune=zerolatency speed-preset=ultrafast ! mp4mux'
    else:
        encode = 'videoconvert ! vp8enc deadline=1 cpu-used=4 ! webmmux'

    pipeline = (
        f'pipewiresrc path={pw_node_id} do-timestamp=true keepalive-time=1000 ! '
        f'videorate ! video/x-raw,framerate={fps}/1 ! '
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
        SRU_LOG_ERROR("ensureRecorderScript: failed to write " << scriptPath);
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
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec))
        {
            return canonical.string();
        }
    }

    // Fallback: search PATH manually (no shell invocation)
    std::string pathResult = findInPath("ffmpeg");
    if (!pathResult.empty())
    {
        return pathResult;
    }

    SRU_LOG_WARN("FFmpeg not found in candidates or PATH");
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
        SRU_LOG_ERROR("startRecording: outputPath is empty");
        return {};
    }

    // Ensure output directory exists
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(config.outputPath).parent_path(), ec);

    // Portal + GStreamer records into a temp .webm, then we convert to target format.
    std::string tempWebm = std::filesystem::path(config.outputPath).parent_path()
        / ("portal_rec_" + std::to_string(getpid()) + ".webm");
    session.waylandTempPath = tempWebm;

    // Write the Python portal recorder script
    std::string scriptPath = ensureRecorderScript();
    if (scriptPath.empty())
    {
        SRU_LOG_ERROR("startRecording: failed to write portal recorder script");
        session.isWaylandScreencast = false;
        return {};
    }

    // Create a pipe for reading the child's stdout (protocol messages)
    int pipeFds[2];
    if (pipe(pipeFds) != 0)
    {
        SRU_LOG_ERROR("startRecording: pipe() failed: " << std::strerror(errno));
        session.isWaylandScreencast = false;
        return {};
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
        SRU_LOG_ERROR("startRecording: posix_spawn failed: " << std::strerror(spawnRc));
        close(pipeFds[0]);
        session.isWaylandScreencast = false;
        return {};
    }

    session.pid = pid;
    session.stdinFd = pipeFds[0]; // repurpose stdinFd as read-end of stdout pipe

    // Store region info for post-recording FFmpeg crop
    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        session.isRegion = true;
        session.regionX = config.regionX;
        session.regionY = config.regionY;
        session.regionW = config.regionW;
        session.regionH = config.regionH;
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
            SRU_LOG_ERROR("startRecording: portal script error: " << accumulated);
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
        SRU_LOG_ERROR("startRecording: portal recorder did not start within timeout");
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
        close(pipeFds[0]);
        session.pid = -1;
        session.isWaylandScreencast = false;
        return {};
    }

    SRU_LOG_INFO("startRecording: portal recorder started, pid=" << pid
                 << " output=" << config.outputPath);
    return session;
}

RecordingResult ScreenRecordingUtils_Linux::stopRecording(RecordingSession& session)
{
    RecordingResult result;
    result.outputPath = session.outputPath;

    if (!session.isValid())
    {
        result.errorMessage = "Invalid session";
        SRU_LOG_ERROR("stopRecording: " << result.errorMessage);
        return result;
    }

    // Send SIGTERM to the Python script; it will SIGINT gst-launch → EOS → clean file
    if (session.pid > 0)
    {
        SRU_LOG_INFO("stopRecording: sending SIGTERM to pid " << session.pid);
        kill(static_cast<pid_t>(session.pid), SIGTERM);

        // Wait for exit (max 8 seconds — GStreamer EOS finalization can take a moment)
        int status = 0;
        std::string killMsg;
        waitForChildWithTimeout(static_cast<pid_t>(session.pid), status, 8000, killMsg);

        if (!killMsg.empty())
        {
            SRU_LOG_WARN("stopRecording: " << killMsg);
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
        auto sz = std::filesystem::file_size(session.waylandTempPath, ec);
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
    if (!std::filesystem::is_regular_file(session.waylandTempPath, ec))
    {
        result.errorMessage = "Recording file not produced";
        SRU_LOG_ERROR("stopRecording: " << result.errorMessage);
        return result;
    }

    // Verify the file has actual video content
    {
        std::error_code ec2;
        auto sz = std::filesystem::file_size(session.waylandTempPath, ec2);
        if (ec2 || sz < 1024)
        {
            std::filesystem::remove(session.waylandTempPath, ec2);
            result.errorMessage = "Recording too short — no video frames captured";
            SRU_LOG_ERROR("stopRecording: " << result.errorMessage);
            return result;
        }
    }

    // If the requested format is WebM and no region crop, just rename the temp file
    std::string targetExt = std::filesystem::path(session.outputPath).extension().string();
    if (targetExt == ".webm" && !session.isRegion)
    {
        std::filesystem::rename(session.waylandTempPath, session.outputPath, ec);
        if (ec)
        {
            std::filesystem::copy_file(session.waylandTempPath, session.outputPath,
                          std::filesystem::copy_options::overwrite_existing, ec);
            std::filesystem::remove(session.waylandTempPath, ec);
        }
        if (std::filesystem::is_regular_file(session.outputPath, ec))
        {
            result.success = true;
            SRU_LOG_INFO("stopRecording: recording saved to " << session.outputPath);
            return result;
        }
        result.errorMessage = "Failed to move recording file";
        SRU_LOG_ERROR("stopRecording: " << result.errorMessage);
        return result;
    }

    // Convert webm → target format (and apply region crop if needed)
    int cx = 0, cy = 0, cw = 0, ch = 0;
    if (session.isRegion && session.regionW > 0 && session.regionH > 0)
    {
        cx = session.regionX;
        cy = session.regionY;
        cw = session.regionW;
        ch = session.regionH;
    }

    bool ok = convertVideoFormat(session.ffmpegPath,
                                 session.waylandTempPath,
                                 session.outputPath,
                                 cx, cy, cw, ch);
    std::filesystem::remove(session.waylandTempPath, ec);

    if (ok)
    {
        result.success = true;
        SRU_LOG_INFO("stopRecording: recording converted and saved to " << session.outputPath);
        return result;
    }
    result.errorMessage = "Format conversion from WebM failed";
    SRU_LOG_ERROR("stopRecording: " << result.errorMessage);
    return result;
}

bool ScreenRecordingUtils_Linux::pauseRecording(const RecordingSession& session)
{
    // Pause GStreamer pipeline by sending SIGSTOP to the Python process group
    if (session.pid > 0)
    {
        SRU_LOG_INFO("pauseRecording: sending SIGSTOP to pid " << session.pid);
        kill(static_cast<pid_t>(session.pid), SIGSTOP);
        return true;
    }
    SRU_LOG_WARN("pauseRecording: invalid pid");
    return false;
}

bool ScreenRecordingUtils_Linux::resumeRecording(const RecordingSession& session)
{
    // Resume GStreamer pipeline by sending SIGCONT
    if (session.pid > 0)
    {
        SRU_LOG_INFO("resumeRecording: sending SIGCONT to pid " << session.pid);
        kill(static_cast<pid_t>(session.pid), SIGCONT);
        return true;
    }
    SRU_LOG_WARN("resumeRecording: invalid pid");
    return false;
}

bool ScreenRecordingUtils_Linux::convertToGif(const std::string& ffmpegPath,
                                              const std::string& inputPath,
                                              const std::string& outputPath,
                                              int fps)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
    {
        SRU_LOG_ERROR("convertToGif: empty path argument");
        return false;
    }

    std::string filterComplex = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    std::vector<std::string> args = {
        ffmpegPath, "-y", "-i", inputPath,
        "-filter_complex", filterComplex,
        outputPath
    };

    SRU_LOG_INFO("convertToGif: " << inputPath << " -> " << outputPath);
    constexpr int kGifTimeoutMs = 120000;
    bool ok = spawnAndWait(ffmpegPath, args, kGifTimeoutMs);

    std::error_code ec;
    auto sz = std::filesystem::file_size(outputPath, ec);
    if (!ok || ec || sz == 0)
    {
        SRU_LOG_ERROR("convertToGif: failed or output empty");
        return false;
    }
    return true;
}

} // namespace ucf::utilities::screenrecording

#endif // __linux__
