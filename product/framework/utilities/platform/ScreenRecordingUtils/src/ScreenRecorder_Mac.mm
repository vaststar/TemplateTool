#include "ScreenRecorder_Mac.h"

#ifdef __APPLE__

#include "LoggerDefine.h"

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>
#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridgeCallback.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include <dlfcn.h>
#include <climits>
#include <signal.h>
#include <unistd.h>

#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreAudio/CoreAudio.h>
#include <AVFoundation/AVFoundation.h>

namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers
// ============================================================================

static inline int alignToEven(int v)
{
    return std::max(2, v & ~1);
}

static std::string findInPath(const std::string& name)
{
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv) return {};

    std::string pathStr(pathEnv);
    std::string::size_type start = 0;
    while (start < pathStr.size())
    {
        auto end = pathStr.find(':', start);
        if (end == std::string::npos) end = pathStr.size();

        std::string dir = pathStr.substr(start, end - start);
        if (!dir.empty())
        {
            auto candidate = std::filesystem::path(dir) / name;
            std::error_code ec;
            if (std::filesystem::is_regular_file(candidate, ec) && access(candidate.c_str(), X_OK) == 0)
            {
                auto canonical = std::filesystem::canonical(candidate, ec);
                if (!ec) return canonical.string();
            }
        }
        start = end + 1;
    }
    return {};
}

// ============================================================================
// Library self-location
// ============================================================================

std::string ScreenRecorder_Mac::getLibraryDirectory()
{
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(&ScreenRecorder_Mac::getLibraryDirectory), &info) == 0
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

std::string ScreenRecorder_Mac::findFFmpegPath()
{
    std::string libDir = getLibraryDirectory();
    if (libDir.empty())
        return findInPath("ffmpeg");

    for (const auto& candidate : {
        libDir + "/ffmpeg",
        libDir + "/../MacOS/ffmpeg",
        libDir + "/../Resources/ffmpeg",
        libDir + "/../../bin/ffmpeg",
        std::string("/opt/homebrew/bin/ffmpeg"),
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

std::string ScreenRecorder_Mac::findFFmpegPath(const std::string& appDir)
{
    for (const auto& candidate : {
        appDir + "/../../../ffmpeg",
        appDir + "/ffmpeg",
        appDir + "/../Resources/ffmpeg",
        std::string("/opt/homebrew/bin/ffmpeg"),
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
// Permissions
// ============================================================================

bool ScreenRecorder_Mac::hasScreenRecordingPermission()
{
    if (__builtin_available(macOS 10.15, *))
        return CGPreflightScreenCaptureAccess();
    return true;
}

bool ScreenRecorder_Mac::hasMicrophonePermission()
{
    if (@available(macOS 10.14, *))
        return [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio]
               == AVAuthorizationStatusAuthorized;
    return true;
}

void ScreenRecorder_Mac::requestMicrophonePermission(std::function<void(bool)> callback)
{
    if (@available(macOS 10.14, *))
    {
        AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
        if (status == AVAuthorizationStatusAuthorized)
        {
            if (callback) callback(true);
            return;
        }
        if (status == AVAuthorizationStatusNotDetermined)
        {
            [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted) {
                if (callback) callback(granted);
            }];
            return;
        }
        if (callback) callback(false);
        return;
    }
    if (callback) callback(true);
}

// ============================================================================
// Audio device enumeration
// ============================================================================

std::vector<AudioDeviceInfo> ScreenRecorder_Mac::enumerateAudioDevices()
{
    std::vector<AudioDeviceInfo> devices;

    AudioObjectPropertyAddress propAddr = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    UInt32 dataSize = 0;
    OSStatus status = AudioObjectGetPropertyDataSize(
        kAudioObjectSystemObject, &propAddr, 0, nullptr, &dataSize);
    if (status != noErr || dataSize == 0) return devices;

    UInt32 deviceCount = dataSize / sizeof(AudioDeviceID);
    std::vector<AudioDeviceID> deviceIDs(deviceCount);
    status = AudioObjectGetPropertyData(
        kAudioObjectSystemObject, &propAddr, 0, nullptr, &dataSize, deviceIDs.data());
    if (status != noErr) return devices;

    for (AudioDeviceID devId : deviceIDs)
    {
        // Check input streams — avfoundation requires input streams
        AudioObjectPropertyAddress inputAddr = {
            kAudioDevicePropertyStreams, kAudioDevicePropertyScopeInput, kAudioObjectPropertyElementMain
        };
        UInt32 inputSize = 0;
        AudioObjectGetPropertyDataSize(devId, &inputAddr, 0, nullptr, &inputSize);
        if (inputSize == 0) continue;

        // Devices with output streams too are virtual loopback (BlackHole, Soundflower)
        AudioObjectPropertyAddress outputAddr = {
            kAudioDevicePropertyStreams, kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMain
        };
        UInt32 outputSize = 0;
        AudioObjectGetPropertyDataSize(devId, &outputAddr, 0, nullptr, &outputSize);
        bool hasOutput = (outputSize > 0);

        // Get device name
        AudioObjectPropertyAddress nameAddr = {
            kAudioObjectPropertyName, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain
        };
        CFStringRef cfName = nullptr;
        UInt32 nameSize = sizeof(CFStringRef);
        if (AudioObjectGetPropertyData(devId, &nameAddr, 0, nullptr, &nameSize, &cfName) != noErr || !cfName)
            continue;

        char nameBuf[256];
        CFStringGetCString(cfName, nameBuf, sizeof(nameBuf), kCFStringEncodingUTF8);
        CFRelease(cfName);
        std::string name(nameBuf);

        if (hasOutput)
            devices.push_back({name, name, false, AudioDeviceType::LoopbackCapture});
        else
            devices.push_back({name, name, true, AudioDeviceType::Microphone});
    }
    return devices;
}

// ============================================================================
// GIF conversion
// ============================================================================

bool ScreenRecorder_Mac::convertToGif(const std::string& ffmpegPath,
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
    return result.exitCode == 0 && std::filesystem::is_regular_file(outputPath, ec);
}

// ============================================================================
// Construction / Destruction
// ============================================================================

ScreenRecorder_Mac::ScreenRecorder_Mac() = default;

ScreenRecorder_Mac::~ScreenRecorder_Mac()
{
    if (m_active.load()) stop();
}

// ============================================================================
// Recording — start
// ============================================================================

bool ScreenRecorder_Mac::start(const RecordingConfig& config)
{
    if (m_active.load())
        return false;

    m_outputPath = config.outputPath;
    if (config.ffmpegPath.empty() || config.outputPath.empty())
        return false;

    if (!hasScreenRecordingPermission())
    {
        SRU_LOG_ERROR("start: screen recording permission not granted");
        return false;
    }

    // Determine audio inputs
    std::string audioInputName = "none";
    bool needSecondInput = false;
    std::string secondAudioInputName;

    switch (config.audioMode)
    {
    case AudioCaptureMode::None:
        audioInputName = "none"; break;
    case AudioCaptureMode::Microphone:
        audioInputName = config.micDevice.empty() ? "default" : config.micDevice; break;
    case AudioCaptureMode::SystemAudio:
        audioInputName = config.systemAudioDevice.empty() ? "none" : config.systemAudioDevice; break;
    case AudioCaptureMode::MicAndSystem:
        audioInputName = config.micDevice.empty() ? "default" : config.micDevice;
        if (!config.systemAudioDevice.empty())
        {
            needSecondInput = true;
            secondAudioInputName = config.systemAudioDevice;
        }
        break;
    }

    std::string inputSpec = "Capture screen " + std::to_string(config.displayIndex) + ":" + audioInputName;

    // Build FFmpeg arguments
    std::vector<std::string> args = {"-y",
        "-thread_queue_size", "512",
        "-f", "avfoundation",
        "-framerate", std::to_string(config.fps),
        "-capture_cursor", "1",
        "-use_wallclock_as_timestamps", "1",
        "-probesize", "32",
        "-i", inputSpec};

    if (needSecondInput)
        args.insert(args.end(), {"-thread_queue_size", "512", "-f", "avfoundation",
                                 "-i", "none:" + secondAudioInputName});

    args.insert(args.end(), {"-vsync", "0"});

    // Region crop + audio mix filters
    std::string videoFilter;
    if (config.isRegion && config.regionW > 0 && config.regionH > 0)
    {
        videoFilter = "crop=" + std::to_string(alignToEven(config.regionW)) + ":"
                    + std::to_string(alignToEven(config.regionH)) + ":"
                    + std::to_string(config.regionX) + ":" + std::to_string(config.regionY);
    }

    if (needSecondInput)
    {
        std::string fc;
        if (!videoFilter.empty())
        {
            fc = "[0:v]" + videoFilter + "[vout];"
                "[0:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a0];"
                "[1:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a1];"
                "[a0][a1]amix=inputs=2:duration=first:normalize=0[aout]";
            args.insert(args.end(), {"-filter_complex", fc, "-map", "[vout]", "-map", "[aout]"});
        }
        else
        {
            fc = "[0:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a0];"
                 "[1:a]aresample=48000,aformat=sample_fmts=fltp:channel_layouts=stereo[a1];"
                 "[a0][a1]amix=inputs=2:duration=first:normalize=0[aout]";
            args.insert(args.end(), {"-filter_complex", fc, "-map", "0:v", "-map", "[aout]"});
        }
    }
    else if (!videoFilter.empty())
    {
        args.insert(args.end(), {"-vf", videoFilter});
    }

    // Output codec
    if (config.videoFormat == "webm")
    {
        args.insert(args.end(), {"-c:v", "libvpx-vp9", "-b:v", "2M"});
    }
    else
    {
        args.insert(args.end(), {"-c:v", "libx264", "-preset", "ultrafast",
                                 "-tune", "zerolatency", "-pix_fmt", "yuv420p"});
        if (config.videoFormat == "mp4" || config.videoFormat == "mov")
            args.insert(args.end(), {"-video_track_timescale", "600"});
    }

    if (config.audioMode != AudioCaptureMode::None)
    {
        args.insert(args.end(), {"-ar", "48000", "-ac", "2"});
        if (config.videoFormat == "webm")
            args.insert(args.end(), {"-c:a", "libopus", "-b:a", "128k"});
        else
            args.insert(args.end(), {"-c:a", "aac", "-b:a", "128k"});
    }

    args.push_back(config.outputPath);

    // Launch FFmpeg
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
        m_process.reset();
        return false;
    }

    m_active.store(true);
    SRU_LOG_INFO("start: ffmpeg pid=" << m_process->processPid()
                 << " output=" << config.outputPath);
    return true;
}

// ============================================================================
// Recording — stop
// ============================================================================

RecordingResult ScreenRecorder_Mac::stop()
{
    RecordingResult result;
    result.outputPath = m_outputPath;

    if (!m_active.load())
    {
        result.errorMessage = "Not recording";
        return result;
    }

    if (m_process)
    {
        m_process->writeToStdin("q\n");
        m_process->closeStdin();
    }

    if (m_process)
    {
        constexpr int kTimeoutMs = 10000, kPollMs = 100;
        int waited = 0;
        while (m_process->isRunning() && waited < kTimeoutMs)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(kPollMs));
            waited += kPollMs;
        }
        if (m_process->isRunning())
        {
            SRU_LOG_WARN("stop: FFmpeg did not exit in " << kTimeoutMs << "ms, force-killing");
            m_process->stop();
            result.errorMessage = "FFmpeg force killed";
        }
        m_process.reset();
    }

    m_active.store(false);

    std::error_code ec;
    if (std::filesystem::is_regular_file(result.outputPath, ec))
        result.success = true;
    else if (result.errorMessage.empty())
        result.errorMessage = "Output file not found: " + result.outputPath;

    return result;
}

// ============================================================================
// Recording — pause / resume
// ============================================================================

bool ScreenRecorder_Mac::pause()
{
    if (!m_active.load() || !m_process) return false;
    return kill(static_cast<pid_t>(m_process->processPid()), SIGSTOP) == 0;
}

bool ScreenRecorder_Mac::resume()
{
    if (!m_active.load() || !m_process) return false;
    return kill(static_cast<pid_t>(m_process->processPid()), SIGCONT) == 0;
}

// ============================================================================
// State queries
// ============================================================================

bool ScreenRecorder_Mac::isActive() const { return m_active.load(); }
std::string ScreenRecorder_Mac::outputPath() const { return m_outputPath; }

} // namespace ucf::utilities::screenrecording

#endif // __APPLE__
