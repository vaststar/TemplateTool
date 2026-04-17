#include "ScreenRecorder_Mac.h"

#ifdef __APPLE__

#include "LoggerDefine.h"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include <dlfcn.h>
#include <climits>
#include <unistd.h>

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>

#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreAudio/CoreAudio.h>
#include <AVFoundation/AVFoundation.h>

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

// ============================================================================
// Library Self-Location
// ============================================================================

std::string ScreenRecorder_Mac::getLibraryDirectory()
{
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(&ScreenRecorder_Mac::getLibraryDirectory), &info) == 0
        || info.dli_fname == nullptr)
    {
        SRU_LOG_ERROR("dladdr failed for ScreenRecorder_Mac");
        return {};
    }

    std::string fullPath = info.dli_fname;
    if (!fullPath.empty() && fullPath[0] != '/')
    {
        char resolved[PATH_MAX] = {};
        if (realpath(fullPath.c_str(), resolved))
        {
            fullPath = resolved;
        }
    }

    auto lastSep = fullPath.rfind('/');
    if (lastSep == std::string::npos)
    {
        return ".";
    }
    return fullPath.substr(0, lastSep);
}

// ============================================================================
// FFmpeg Discovery
// ============================================================================

std::string ScreenRecorder_Mac::findFFmpegPath()
{
    std::string libDir = getLibraryDirectory();
    if (libDir.empty())
    {
        SRU_LOG_WARN("Cannot determine library directory for FFmpeg auto-discovery");
        return findInPath("ffmpeg");
    }

    // Typical layouts:
    //   Build:   <build>/bin/libScreenRecordingUtils.dylib  →  ffmpeg at <build>/bin/ffmpeg
    //   Bundle:  .../mainEntry.app/Contents/Frameworks/libScreenRecordingUtils.dylib
    //            →  ffmpeg at .../mainEntry.app/Contents/MacOS/ffmpeg
    //            or .../mainEntry.app/bin/ffmpeg
    std::vector<std::string> candidates = {
        libDir + "/ffmpeg",                     // same dir (build output: bin/)
        libDir + "/../MacOS/ffmpeg",            // inside .app bundle
        libDir + "/../Resources/ffmpeg",        // .app Resources
        libDir + "/../../bin/ffmpeg",           // one level up
        "/opt/homebrew/bin/ffmpeg",
        "/usr/local/bin/ffmpeg"
    };

    for (const auto& candidate : candidates)
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec) && access(canonical.c_str(), X_OK) == 0)
        {
            SRU_LOG_INFO("FFmpeg auto-discovered at: " << canonical.string());
            return canonical.string();
        }
    }

    std::string pathResult = findInPath("ffmpeg");
    if (!pathResult.empty())
    {
        return pathResult;
    }

    SRU_LOG_WARN("FFmpeg not found via auto-discovery");
    return {};
}

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
            devices.push_back({name, name, false, AudioDeviceType::LoopbackCapture});
        }
        else
        {
            // Input-only device — microphone.
            devices.push_back({name, name, true, AudioDeviceType::Microphone});
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

    std::string filterComplex = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    ucf::utilities::ProcessBridgeConfig config;
    config.executablePath = ffmpegPath;
    config.arguments = {"-y", "-i", inputPath, "-filter_complex", filterComplex, outputPath};
    config.stopTimeoutMs = 120000;

    auto result = ucf::utilities::IProcessBridge::run(config);

    if (result.timedOut)
    {
        SRU_LOG_ERROR("convertToGif: FFmpeg timed out");
        return false;
    }

    std::error_code ec;
    bool ok = result.exitCode == 0 && std::filesystem::is_regular_file(outputPath, ec);
    if (!ok)
    {
        SRU_LOG_ERROR("convertToGif: ffmpeg exited with status " << result.exitCode);
    }
    return ok;
}

} // namespace ucf::utilities::screenrecording

#endif // __APPLE__
