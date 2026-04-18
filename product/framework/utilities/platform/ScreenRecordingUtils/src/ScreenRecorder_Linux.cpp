#include "ScreenRecorder_Linux.h"

#ifdef __linux__

#include "LoggerDefine.h"

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <dlfcn.h>
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
// Helpers
// ============================================================================

static inline int alignToEven(int v)
{
    return std::max(2, v & ~1);
}

static inline void setCloseOnExec(int fd)
{
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
}

static std::string findInPath(const std::string& name)
{
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv) return {};

    std::istringstream stream(pathEnv);
    std::string dir;
    while (std::getline(stream, dir, ':'))
    {
        if (dir.empty()) continue;
        auto candidate = std::filesystem::path(dir) / name;
        std::error_code ec;
        if (std::filesystem::is_regular_file(candidate, ec) && access(candidate.c_str(), X_OK) == 0)
        {
            auto canonical = std::filesystem::canonical(candidate, ec);
            if (!ec) return canonical.string();
        }
    }
    return {};
}

/// Wait for child process with timeout, escalating SIGINT → SIGKILL.
static bool waitForChildWithTimeout(pid_t pid, int& status, int timeoutMs, std::string& errorMsg)
{
    constexpr int kPollMs = 100;

    int elapsed = 0;
    while (elapsed < timeoutMs)
    {
        if (waitpid(pid, &status, WNOHANG) != 0) return true;
        usleep(kPollMs * 1000);
        elapsed += kPollMs;
    }

    SRU_LOG_WARN("Child " << pid << " did not exit in " << timeoutMs << "ms, sending SIGINT");
    kill(pid, SIGINT);

    constexpr int kIntTimeoutMs = 5000;
    elapsed = 0;
    while (elapsed < kIntTimeoutMs)
    {
        if (waitpid(pid, &status, WNOHANG) != 0) return true;
        usleep(kPollMs * 1000);
        elapsed += kPollMs;
    }

    SRU_LOG_ERROR("Child " << pid << " did not respond to SIGINT, sending SIGKILL");
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    errorMsg = "Process did not exit in time, force killed";
    return false;
}

/// Convert video format via FFmpeg, optionally cropping to region.
static bool convertVideoFormat(const std::string& ffmpegPath,
                               const std::string& inputPath,
                               const std::string& outputPath,
                               int cropX = 0, int cropY = 0,
                               int cropW = 0, int cropH = 0)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
        return false;

    std::error_code ec;
    if (!std::filesystem::is_regular_file(inputPath, ec))
        return false;

    std::string ext = std::filesystem::path(outputPath).extension().string();
    std::vector<std::string> args = {"-y", "-i", inputPath};

    int cw = alignToEven(cropW), ch = alignToEven(cropH);

    if (ext == ".mp4" || ext == ".mov")
    {
        if (cropW > 0 && cropH > 0)
        {
            std::string crop = "crop=" + std::to_string(cw) + ":" + std::to_string(ch)
                             + ":" + std::to_string(cropX) + ":" + std::to_string(cropY)
                             + ",format=yuv420p";
            args.insert(args.end(), {"-vf", crop});
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
            std::string crop = "crop=" + std::to_string(cw) + ":" + std::to_string(ch)
                             + ":" + std::to_string(cropX) + ":" + std::to_string(cropY);
            args.insert(args.end(), {"-vf", crop, "-c:v", "libvpx", "-b:v", "2M"});
        }
        else
        {
            args.insert(args.end(), {"-c", "copy"});
        }
    }

    args.push_back(outputPath);

    SRU_LOG_INFO("convertVideoFormat: " << inputPath << " -> " << outputPath);

    ucf::utilities::ProcessBridgeConfig pbConfig;
    pbConfig.executablePath = ffmpegPath;
    pbConfig.arguments = std::move(args);
    pbConfig.stopTimeoutMs = 120000;

    auto result = ucf::utilities::IProcessBridge::run(pbConfig);
    if (result.timedOut) return false;

    auto sz = std::filesystem::file_size(outputPath, ec);
    return result.exitCode == 0 && !ec && sz > 0;
}

// ============================================================================
// Embedded Python portal recorder script
// ============================================================================

static constexpr const char* kPortalRecorderScript = R"PYTHON(#!/usr/bin/env python3
"""
Portal ScreenCast recorder — xdg-desktop-portal D-Bus API + GStreamer.

Usage: portal_recorder.py <output_path> <fps> <audio_mode> [<mic_device>] [<sys_device>]
  audio_mode: none / microphone / system / mic_and_system

Protocol (stdout): RECORDING | ERROR:<message> | STOPPED
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

    time.sleep(0.3)
    if gst_proc.poll() is not None:
        emit_error(f'gst-launch-1.0 exited immediately (rc={gst_proc.returncode})')
        return

    print('RECORDING', flush=True)

def on_select_sources_response(response, results):
    if response != 0:
        emit_error(f'SelectSources rejected (response={response})')
        return
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
        'types': dbus.UInt32(1),
        'cursor_mode': dbus.UInt32(2),
        'persist_mode': dbus.UInt32(2),
    }
    saved_token = load_restore_token()
    if saved_token:
        select_opts['restore_token'] = saved_token

    screencast_iface.SelectSources(
        session_path,
        dbus.Dictionary(select_opts, signature='sv'),
    )

def stop_recording(signum=None, frame=None):
    if gst_proc and gst_proc.poll() is None:
        gst_proc.send_signal(signal.SIGINT)
        try:
            gst_proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            gst_proc.kill()
            gst_proc.wait()
    print('STOPPED', flush=True)
    loop.quit()

signal.signal(signal.SIGTERM, stop_recording)
signal.signal(signal.SIGINT, stop_recording)

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

static std::string ensureRecorderScript()
{
    std::string scriptPath = "/tmp/portal_recorder_" + std::to_string(getuid()) + ".py";
    std::ofstream ofs(scriptPath, std::ios::trunc);
    if (!ofs.is_open()) return {};
    ofs << kPortalRecorderScript;
    ofs.close();
    chmod(scriptPath.c_str(), 0700);
    return scriptPath;
}

// ============================================================================
// Library self-location
// ============================================================================

std::string ScreenRecorder_Linux::getLibraryDirectory()
{
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(&ScreenRecorder_Linux::getLibraryDirectory), &info) == 0
        || info.dli_fname == nullptr)
        return {};

    std::string fullPath = info.dli_fname;
    if (!fullPath.empty() && fullPath[0] != '/')
    {
        char resolved[PATH_MAX] = {};
        if (realpath(fullPath.c_str(), resolved))
            fullPath = resolved;
    }

    auto lastSep = fullPath.rfind('/');
    return (lastSep != std::string::npos) ? fullPath.substr(0, lastSep) : ".";
}

// ============================================================================
// FFmpeg discovery
// ============================================================================

std::string ScreenRecorder_Linux::findFFmpegPath()
{
    std::string libDir = getLibraryDirectory();
    if (libDir.empty())
        return findInPath("ffmpeg");

    for (const auto& candidate : {
        libDir + "/ffmpeg",
        libDir + "/../bin/ffmpeg",
        libDir + "/../lib/ffmpeg",
        std::string("/usr/bin/ffmpeg"),
        std::string("/usr/local/bin/ffmpeg")})
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec) && access(canonical.c_str(), X_OK) == 0)
        {
            SRU_LOG_INFO("FFmpeg auto-discovered at: " << canonical.string());
            return canonical.string();
        }
    }
    return findInPath("ffmpeg");
}

std::string ScreenRecorder_Linux::findFFmpegPath(const std::string& appDir)
{
    for (const auto& candidate : {
        appDir + "/ffmpeg",
        appDir + "/../lib/ffmpeg",
        std::string("/usr/bin/ffmpeg"),
        std::string("/usr/local/bin/ffmpeg")})
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec))
            return canonical.string();
    }
    return findInPath("ffmpeg");
}

// ============================================================================
// Audio device enumeration (PulseAudio via pactl)
// ============================================================================

std::vector<AudioDeviceInfo> ScreenRecorder_Linux::enumerateAudioDevices()
{
    std::vector<AudioDeviceInfo> devices;

    auto parseDevices = [&](bool isInput) {
        int pfd[2];
        if (pipe(pfd) != 0) return;

        pid_t pid = fork();
        if (pid < 0) { close(pfd[0]); close(pfd[1]); return; }

        if (pid == 0)
        {
            close(pfd[0]);
            dup2(pfd[1], STDOUT_FILENO);
            close(pfd[1]);
            int devNull = open("/dev/null", O_WRONLY);
            if (devNull >= 0) { dup2(devNull, STDERR_FILENO); close(devNull); }

            if (isInput)
                execlp("pactl", "pactl", "list", "sources", "short", nullptr);
            else
                execlp("pactl", "pactl", "list", "sinks", "short", nullptr);
            _exit(1);
        }
        close(pfd[1]);

        std::string output;
        char buf[1024];
        ssize_t n;
        while ((n = read(pfd[0], buf, sizeof(buf) - 1)) > 0)
        {
            buf[n] = '\0';
            output += buf;
        }
        close(pfd[0]);
        waitpid(pid, nullptr, 0);

        // Parse tab-separated: index\tname\tmodule\tsample_spec\tstate
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line))
        {
            if (line.empty()) continue;
            auto tabPos = line.find('\t');
            if (tabPos == std::string::npos) continue;
            auto nameStart = tabPos + 1;
            auto nameEnd = line.find('\t', nameStart);
            if (nameEnd == std::string::npos) nameEnd = line.size();

            std::string name = line.substr(nameStart, nameEnd - nameStart);
            if (name.empty()) continue;

            if (isInput && name.find(".monitor") != std::string::npos) continue;
            if (!isInput && name.find(".monitor") == std::string::npos) continue;

            auto devType = isInput ? AudioDeviceType::Microphone : AudioDeviceType::LoopbackCapture;
            devices.push_back({name, name, isInput, devType});
        }
    };

    parseDevices(true);
    parseDevices(false);
    return devices;
}

// ============================================================================
// GIF conversion
// ============================================================================

bool ScreenRecorder_Linux::convertToGif(const std::string& ffmpegPath,
                                        const std::string& inputPath,
                                        const std::string& outputPath,
                                        int fps)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
        return false;

    std::string filter = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    ucf::utilities::ProcessBridgeConfig config;
    config.executablePath = ffmpegPath;
    config.arguments = {"-y", "-i", inputPath, "-filter_complex", filter, outputPath};
    config.stopTimeoutMs = 120000;

    auto result = ucf::utilities::IProcessBridge::run(config);
    if (result.timedOut) { SRU_LOG_ERROR("convertToGif: timed out"); return false; }

    std::error_code ec;
    auto sz = std::filesystem::file_size(outputPath, ec);
    return result.exitCode == 0 && !ec && sz > 0;
}

// ============================================================================
// Construction / Destruction
// ============================================================================

ScreenRecorder_Linux::ScreenRecorder_Linux() = default;

ScreenRecorder_Linux::~ScreenRecorder_Linux()
{
    if (m_active) stop();
}

bool ScreenRecorder_Linux::isActive() const { return m_active; }
std::string ScreenRecorder_Linux::outputPath() const { return m_outputPath; }

// ============================================================================
// Recording — start (Wayland portal + GStreamer)
// ============================================================================

bool ScreenRecorder_Linux::start(const RecordingConfig& config)
{
    if (m_active) return false;

    m_outputPath = config.outputPath;
    m_ffmpegPath = config.ffmpegPath;
    if (config.outputPath.empty()) return false;

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(config.outputPath).parent_path(), ec);

    // Portal + GStreamer records into temp .webm, then convert to target format
    std::string tempWebm = std::filesystem::path(config.outputPath).parent_path()
        / ("portal_rec_" + std::to_string(getpid()) + ".webm");
    m_waylandTempPath = tempWebm;

    std::string scriptPath = ensureRecorderScript();
    if (scriptPath.empty()) return false;

    int pipeFds[2];
    if (pipe(pipeFds) != 0) return false;
    setCloseOnExec(pipeFds[0]);
    setCloseOnExec(pipeFds[1]);

    std::string python3Path = findInPath("python3");
    if (python3Path.empty()) python3Path = "/usr/bin/python3";

    std::string fpsStr = std::to_string(config.fps);

    std::string audioModeStr = "none";
    switch (config.audioMode)
    {
    case AudioCaptureMode::None:         audioModeStr = "none"; break;
    case AudioCaptureMode::Microphone:   audioModeStr = "microphone"; break;
    case AudioCaptureMode::SystemAudio:  audioModeStr = "system"; break;
    case AudioCaptureMode::MicAndSystem: audioModeStr = "mic_and_system"; break;
    }

    posix_spawn_file_actions_t fileActions;
    posix_spawn_file_actions_init(&fileActions);
    posix_spawn_file_actions_adddup2(&fileActions, pipeFds[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&fileActions, pipeFds[0]);
    posix_spawn_file_actions_addclose(&fileActions, pipeFds[1]);
    posix_spawn_file_actions_addopen(&fileActions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);

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
                              &fileActions, nullptr, argv.data(), environ);
    posix_spawn_file_actions_destroy(&fileActions);
    close(pipeFds[1]);

    if (spawnRc != 0)
    {
        close(pipeFds[0]);
        return false;
    }

    m_pid = pid;
    m_stdoutPipeFd = pipeFds[0];

    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        m_isRegion = true;
        m_regionX = config.regionX;
        m_regionY = config.regionY;
        m_regionW = config.regionW;
        m_regionH = config.regionH;
    }

    // Wait for "RECORDING" or "ERROR:" (generous timeout for portal dialog)
    constexpr int kStartTimeoutMs = 30000;
    constexpr int kPollChunkMs = 200;

    struct pollfd pfd{};
    pfd.fd = pipeFds[0];
    pfd.events = POLLIN;

    std::string accumulated;
    bool started = false;
    int elapsed = 0;

    while (elapsed < kStartTimeoutMs)
    {
        int ret = poll(&pfd, 1, kPollChunkMs);
        elapsed += kPollChunkMs;

        if (ret > 0 && (pfd.revents & POLLIN))
        {
            char buf[256];
            ssize_t n = read(pipeFds[0], buf, sizeof(buf) - 1);
            if (n > 0) { buf[n] = '\0'; accumulated += buf; }
            else if (n == 0) break;
        }
        else if (ret > 0 && (pfd.revents & (POLLHUP | POLLERR)))
        {
            break;
        }

        if (accumulated.find("RECORDING") != std::string::npos) { started = true; break; }
        if (accumulated.find("ERROR:") != std::string::npos) break;

        int status = 0;
        if (waitpid(pid, &status, WNOHANG) != 0) break;
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
    SRU_LOG_INFO("start: portal recorder pid=" << pid << " output=" << config.outputPath);
    return true;
}

// ============================================================================
// Recording — stop
// ============================================================================

RecordingResult ScreenRecorder_Linux::stop()
{
    RecordingResult result;
    result.outputPath = m_outputPath;

    if (!m_active || m_pid <= 0)
    {
        result.errorMessage = "No active recording";
        return result;
    }

    kill(static_cast<pid_t>(m_pid), SIGTERM);

    int status = 0;
    std::string killMsg;
    waitForChildWithTimeout(static_cast<pid_t>(m_pid), status, 8000, killMsg);
    if (!killMsg.empty()) SRU_LOG_WARN("stop: " << killMsg);

    if (m_stdoutPipeFd >= 0) { close(m_stdoutPipeFd); m_stdoutPipeFd = -1; }
    m_pid = -1;
    m_active = false;

    // Wait for file to stabilize (GStreamer may still be flushing)
    constexpr int kMaxStabilizeMs = 3000, kPollIntervalMs = 200;
    int waited = 0;
    std::uintmax_t lastSize = 0;
    int stableCount = 0;
    while (waited < kMaxStabilizeMs)
    {
        usleep(kPollIntervalMs * 1000);
        waited += kPollIntervalMs;
        std::error_code ec;
        auto sz = std::filesystem::file_size(m_waylandTempPath, ec);
        if (ec) continue;
        if (sz > 0 && sz == lastSize) { if (++stableCount >= 2) break; }
        else stableCount = 0;
        lastSize = sz;
    }

    std::error_code ec;
    if (!std::filesystem::is_regular_file(m_waylandTempPath, ec))
    {
        result.errorMessage = "Recording file not produced";
        return result;
    }

    {
        auto sz = std::filesystem::file_size(m_waylandTempPath, ec);
        if (ec || sz < 1024)
        {
            std::filesystem::remove(m_waylandTempPath, ec);
            result.errorMessage = "Recording too short — no video frames captured";
            return result;
        }
    }

    // If target is .webm with no region crop, just rename
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
            return result;
        }
        result.errorMessage = "Failed to move recording file";
        return result;
    }

    // Convert webm → target format (with optional region crop)
    int cx = 0, cy = 0, cw = 0, ch = 0;
    if (m_isRegion && m_regionW > 0 && m_regionH > 0)
    { cx = m_regionX; cy = m_regionY; cw = m_regionW; ch = m_regionH; }

    bool ok = convertVideoFormat(m_ffmpegPath, m_waylandTempPath, m_outputPath, cx, cy, cw, ch);
    std::filesystem::remove(m_waylandTempPath, ec);

    if (ok) { result.success = true; return result; }
    result.errorMessage = "Format conversion from WebM failed";
    return result;
}

// ============================================================================
// Recording — pause / resume
// ============================================================================

bool ScreenRecorder_Linux::pause()
{
    if (m_pid <= 0) return false;
    return kill(static_cast<pid_t>(m_pid), SIGSTOP) == 0;
}

bool ScreenRecorder_Linux::resume()
{
    if (m_pid <= 0) return false;
    return kill(static_cast<pid_t>(m_pid), SIGCONT) == 0;
}

} // namespace ucf::utilities::screenrecording

#endif // __linux__
