#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities::screenrecording {

// ============================================================================
// Data structures
// ============================================================================

/**
 * @brief Audio capture mode for screen recording
 */
enum class AudioCaptureMode
{
    None,           ///< No audio (current default)
    Microphone,     ///< Microphone input only
    SystemAudio,    ///< System/desktop audio only (loopback)
    MicAndSystem    ///< Mix microphone + system audio
};

/**
 * @brief Information about an available audio device
 */
struct Utilities_EXPORT AudioDeviceInfo
{
    std::string id;           ///< Platform-specific device identifier
    std::string displayName;  ///< User-friendly display name
    bool isInput = true;      ///< true = microphone/input, false = output/loopback
};

/**
 * @brief Configuration for starting a recording session
 */
struct Utilities_EXPORT RecordingConfig
{
    std::string ffmpegPath;             ///< Absolute path to ffmpeg binary
    std::string outputPath;             ///< Full output file path (e.g. /path/recording.mp4)
    std::string videoFormat = "mp4";    ///< mp4 / mov / webm
    int fps = 30;                       ///< Frames per second
    int displayIndex = 0;              ///< Display to capture (fullscreen)
    // Region capture
    int regionX = 0, regionY = 0;
    int regionW = 0, regionH = 0;
    bool isRegion = false;              ///< true = region capture, false = fullscreen
    // Audio capture
    AudioCaptureMode audioMode = AudioCaptureMode::None;
    std::string micDevice;              ///< Microphone device id (empty = system default)
    std::string systemAudioDevice;      ///< System audio device id (e.g. BlackHole, Stereo Mix)
};

/**
 * @brief Handle for an active recording session
 */
struct Utilities_EXPORT RecordingSession
{
    int64_t pid = -1;                   ///< Child process PID (posix) / HANDLE (Win)
    int stdinFd = -1;                   ///< Write-end of stdin pipe (to send 'q')
    std::string outputPath;

    // Wayland-specific (xdg-desktop-portal ScreenCast + GStreamer)
    bool isWaylandScreencast = false;   ///< True if using portal ScreenCast
    std::string waylandTempPath;        ///< Temp WebM path recorded by GStreamer
    std::string ffmpegPath;             ///< FFmpeg path for post-conversion on Wayland

    // Region crop (applied via FFmpeg after recording stops)
    bool isRegion = false;
    int regionX = 0, regionY = 0;
    int regionW = 0, regionH = 0;

    [[nodiscard]] bool isValid() const
    {
        return pid > 0 || isWaylandScreencast;
    }
};

/**
 * @brief Result returned after stopping a recording
 */
struct Utilities_EXPORT RecordingResult
{
    bool success = false;
    std::string outputPath;
    std::string errorMessage;
};

// ============================================================================
// ScreenRecordingUtils — cross-platform FFmpeg-based screen recording
// ============================================================================

/**
 * @brief Platform-specific screen recording utility via FFmpeg subprocess.
 *
 * No Qt dependency — uses POSIX fork/exec (macOS/Linux) or CreateProcess (Windows).
 */
class Utilities_EXPORT ScreenRecordingUtils final
{
public:
    // === FFmpeg Discovery ===

    /// Search platform-specific candidate paths relative to appDir.
    /// @param appDir  The application binary directory (equivalent to QCoreApplication::applicationDirPath())
    /// @return Absolute path to ffmpeg, or empty string if not found.
    static std::string findFFmpegPath(const std::string& appDir);

    /// Convenience wrapper around findFFmpegPath.
    static bool isFFmpegAvailable(const std::string& appDir);

    // === Permission Check ===

    /// Check if the application has screen recording permission (macOS 10.15+).
    /// On Windows and Linux, always returns true.
    /// Does NOT trigger a system permission dialog.
    static bool hasScreenRecordingPermission();

    /// Check if the application has microphone permission (macOS 10.14+).
    /// On Windows and Linux, always returns true.
    /// Pure query — does NOT trigger a system permission dialog.
    static bool hasMicrophonePermission();

    /// Request microphone permission (macOS 10.14+).
    /// Triggers the system permission dialog if status is undetermined.
    /// No-op on Windows and Linux (callback receives true immediately).
    /// @param callback  Called asynchronously with the granted result.
    static void requestMicrophonePermission(std::function<void(bool granted)> callback);

    // === Audio Device Enumeration ===

    /// Enumerate available audio input (microphone) and output (loopback) devices.
    static std::vector<AudioDeviceInfo> enumerateAudioDevices();

    // === Recording Control ===

    /// Start an FFmpeg recording subprocess.
    /// @return A session handle; check isValid() for success.
    static RecordingSession startRecording(const RecordingConfig& config);

    /// Stop a running recording by sending 'q' to FFmpeg's stdin.
    /// Waits for the process to exit (with timeout).
    /// @return Result with output path on success, or error message.
    static RecordingResult stopRecording(RecordingSession& session);

    /// Pause a running recording (SIGSTOP on POSIX, SuspendThread on Windows).
    static bool pauseRecording(const RecordingSession& session);

    /// Resume a paused recording (SIGCONT on POSIX, ResumeThread on Windows).
    static bool resumeRecording(const RecordingSession& session);

    // === Format Conversion ===

    /// Convert a video file to GIF using FFmpeg.
    /// @param ffmpegPath  Path to ffmpeg binary
    /// @param inputPath   Source video file
    /// @param outputPath  Destination .gif file (auto-generated if empty)
    /// @param fps         GIF frame rate
    /// @return true on success
    static bool convertToGif(const std::string& ffmpegPath,
                             const std::string& inputPath,
                             const std::string& outputPath,
                             int fps = 10);
};

} // namespace ucf::utilities::screenrecording
