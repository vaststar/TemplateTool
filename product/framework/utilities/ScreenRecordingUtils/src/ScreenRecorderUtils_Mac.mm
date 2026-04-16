#include "ScreenRecorder_Mac.h"

#ifdef __APPLE__

#include "LoggerDefine.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreAudio/CoreAudio.h>
#include <AVFoundation/AVFoundation.h>

extern char** environ;

namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers
// ============================================================================

/// Search the PATH environment variable for an executable by name.
static std::string findInPath(const std::string& name)
{
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv)
    {
        return {};
    }

    std::string pathStr(pathEnv);
    std::string::size_type start = 0;

    while (start < pathStr.size())
    {
        auto end = pathStr.find(':', start);
        if (end == std::string::npos)
        {
            end = pathStr.size();
        }

        std::string dir = pathStr.substr(start, end - start);
        if (!dir.empty())
        {
            std::filesystem::path candidate = std::filesystem::path(dir) / name;
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
        start = end + 1;
    }
    return {};
}

/// Wait for a child process with timeout, escalating from SIGINT to SIGKILL.
static bool waitForChildWithTimeout(pid_t pid, int& status, int timeoutMs, std::string& errorMsg)
{
    constexpr int pollIntervalMs = 100;

    // Phase 1: wait for voluntary exit
    int elapsed = 0;
    while (elapsed < timeoutMs)
    {
        pid_t wpid = waitpid(pid, &status, WNOHANG);
        if (wpid != 0)
        {
            return true;
        }
        usleep(pollIntervalMs * 1000);
        elapsed += pollIntervalMs;
    }

    // Phase 2: SIGINT for graceful muxer finalization (5 seconds)
    SRU_LOG_WARN("Child process " << pid << " did not exit in " << timeoutMs << "ms, sending SIGINT");
    kill(pid, SIGINT);

    constexpr int intTimeoutMs = 5000;
    elapsed = 0;
    while (elapsed < intTimeoutMs)
    {
        pid_t wpid = waitpid(pid, &status, WNOHANG);
        if (wpid != 0)
        {
            return true;
        }
        usleep(pollIntervalMs * 1000);
        elapsed += pollIntervalMs;
    }

    // Phase 3: SIGKILL as last resort
    SRU_LOG_ERROR("Child process " << pid << " did not respond to SIGINT, sending SIGKILL");
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    errorMsg = "FFmpeg did not exit in time, force killed";
    return false;
}

// ============================================================================
// FFmpeg Discovery
// ============================================================================

std::string ScreenRecorder_Mac::findFFmpegPath(const std::string& appDir)
{
    // On macOS, appDir = .../mainEntry.app/Contents/MacOS
    // ffmpeg sits in .../bin/ffmpeg (next to the .app bundle), so go up 3 levels.
    std::vector<std::string> candidates = {
        appDir + "/../../../ffmpeg",          // build output: bin/ffmpeg (next to .app)
        appDir + "/ffmpeg",                   // flat layout
        appDir + "/../Resources/ffmpeg",      // bundled inside .app
        "/opt/homebrew/bin/ffmpeg",
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
// Permission Check
// ============================================================================

bool ScreenRecorder_Mac::hasScreenRecordingPermission()
{
    if (__builtin_available(macOS 10.15, *))
    {
        return CGPreflightScreenCaptureAccess();
    }
    // Before macOS 10.15, no permission was required.
    return true;
}

// ============================================================================
// Microphone Permission
// ============================================================================

bool ScreenRecorder_Mac::hasMicrophonePermission()
{
    if (@available(macOS 10.14, *))
    {
        return [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio]
               == AVAuthorizationStatusAuthorized;
    }
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
        // Denied or Restricted
        if (callback) callback(false);
        return;
    }
    if (callback) callback(true);
}

// ============================================================================
// Audio Device Enumeration
// ============================================================================

std::vector<AudioDeviceInfo> ScreenRecorder_Mac::enumerateAudioDevices()
{
    std::vector<AudioDeviceInfo> devices;

    // Get all audio devices
    AudioObjectPropertyAddress propAddr = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    UInt32 dataSize = 0;
    OSStatus status = AudioObjectGetPropertyDataSize(
        kAudioObjectSystemObject, &propAddr, 0, nullptr, &dataSize);
    if (status != noErr || dataSize == 0)
    {
        return devices;
    }

    UInt32 deviceCount = dataSize / sizeof(AudioDeviceID);
    std::vector<AudioDeviceID> deviceIDs(deviceCount);
    status = AudioObjectGetPropertyData(
        kAudioObjectSystemObject, &propAddr, 0, nullptr, &dataSize, deviceIDs.data());
    if (status != noErr)
    {
        return devices;
    }

    for (AudioDeviceID devId : deviceIDs)
    {
        // Check if device has input streams
        AudioObjectPropertyAddress inputAddr = {
            kAudioDevicePropertyStreams,
            kAudioDevicePropertyScopeInput,
            kAudioObjectPropertyElementMain
        };
        UInt32 inputSize = 0;
        AudioObjectGetPropertyDataSize(devId, &inputAddr, 0, nullptr, &inputSize);
        bool hasInput = (inputSize > 0);

        // avfoundation can only capture from devices with input streams.
        // Physical speakers (output-only) are NOT capturable.
        // Virtual loopback devices (BlackHole, Soundflower) have input streams.
        if (!hasInput)
        {
            continue;
        }

        // Check if device also has output streams — devices with both are
        // virtual loopback devices suitable for system audio capture.
        AudioObjectPropertyAddress outputAddr = {
            kAudioDevicePropertyStreams,
            kAudioDevicePropertyScopeOutput,
            kAudioObjectPropertyElementMain
        };
        UInt32 outputSize = 0;
        AudioObjectGetPropertyDataSize(devId, &outputAddr, 0, nullptr, &outputSize);
        bool hasOutput = (outputSize > 0);

        // Get device name
        AudioObjectPropertyAddress nameAddr = {
            kAudioObjectPropertyName,
            kAudioObjectPropertyScopeGlobal,
            kAudioObjectPropertyElementMain
        };
        CFStringRef cfName = nullptr;
        UInt32 nameSize = sizeof(CFStringRef);
        status = AudioObjectGetPropertyData(devId, &nameAddr, 0, nullptr, &nameSize, &cfName);
        if (status != noErr || !cfName)
        {
            continue;
        }

        char nameBuf[256];
        CFStringGetCString(cfName, nameBuf, sizeof(nameBuf), kCFStringEncodingUTF8);
        CFRelease(cfName);

        std::string name(nameBuf);

        // Use the device name as the id because FFmpeg avfoundation
        // identifies audio devices by name (not CoreAudio UID).
        if (hasInput && hasOutput)
        {
            // Virtual loopback device (e.g. BlackHole, Soundflower) — usable
            // for system audio capture.
            devices.push_back({name, name, false});
        }
        else
        {
            // Input-only device — microphone.
            devices.push_back({name, name, true});
        }
    }

    return devices;
}

// ============================================================================
// GIF Conversion
// ============================================================================

bool ScreenRecorder_Mac::convertToGif(const std::string& ffmpegPath,
                                            const std::string& inputPath,
                                            const std::string& outputPath,
                                            int fps)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
    {
        SRU_LOG_ERROR("convertToGif: empty path argument");
        return false;
    }

    // Build palette + GIF two-pass command as a single filter_complex
    std::string filterComplex = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    // Build argv
    std::vector<std::string> args = {
        ffmpegPath, "-y", "-i", inputPath,
        "-filter_complex", filterComplex,
        outputPath
    };
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (auto& a : args)
    {
        argv.push_back(a.data());
    }
    argv.push_back(nullptr);

    // Set up file actions: redirect stdout and stderr to /dev/null
    posix_spawn_file_actions_t fileActions;
    posix_spawn_file_actions_init(&fileActions);
    posix_spawn_file_actions_addopen(&fileActions, STDOUT_FILENO,
                                     "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&fileActions, STDERR_FILENO,
                                     "/dev/null", O_WRONLY, 0);

    pid_t pid = 0;
    int spawnResult = posix_spawn(&pid, ffmpegPath.c_str(),
                                  &fileActions, nullptr,
                                  argv.data(), environ);
    posix_spawn_file_actions_destroy(&fileActions);

    if (spawnResult != 0)
    {
        SRU_LOG_ERROR("convertToGif: posix_spawn failed: " << strerror(spawnResult));
        return false;
    }

    // Wait with timeout (120 seconds for potentially long GIF conversions)
    int status = 0;
    std::string killError;
    bool exited = waitForChildWithTimeout(pid, status, 120000, killError);

    if (!exited)
    {
        SRU_LOG_ERROR("convertToGif: " << killError);
        return false;
    }

    std::error_code ec;
    bool ok = WIFEXITED(status) && WEXITSTATUS(status) == 0
              && std::filesystem::is_regular_file(outputPath, ec);
    if (!ok)
    {
        SRU_LOG_ERROR("convertToGif: ffmpeg exited with status "
                      << (WIFEXITED(status) ? WEXITSTATUS(status) : -1));
    }
    return ok;
}

} // namespace ucf::utilities::screenrecording

#endif // __APPLE__
