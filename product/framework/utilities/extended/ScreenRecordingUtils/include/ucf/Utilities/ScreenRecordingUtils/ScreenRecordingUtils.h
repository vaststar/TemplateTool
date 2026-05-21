#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <memory>

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
 * @brief Classification of audio devices by type.
 */
enum class AudioDeviceType
{
    Microphone,         ///< Physical microphone / headset input
    LoopbackCapture,    ///< Virtual loopback device (Stereo Mix, BlackHole, PulseAudio .monitor)
    OutputDevice        ///< Physical output (speakers/headphones) — usable via WASAPI loopback
};

/**
 * @brief Information about an available audio device
 */
struct Utilities_EXPORT AudioDeviceInfo
{
    std::string id;           ///< Platform-specific device identifier
    std::string displayName;  ///< User-friendly display name
    bool isInput = true;      ///< true = microphone/input, false = output/loopback
    AudioDeviceType deviceType = AudioDeviceType::Microphone;
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
    std::string systemAudioDevice;      ///< System audio device id
    AudioDeviceType systemAudioDeviceType = AudioDeviceType::LoopbackCapture;
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
// IScreenRecorder — one recording session as an object with a lifetime
// ============================================================================

/**
 * @brief Interface for a single screen recording session.
 *
 * Each platform provides its own implementation that owns all recording
 * resources (child processes, capture threads, pipes). Resources are
 * cleaned up automatically when the object is destroyed.
 *
 * Usage:
 *   auto recorder = IScreenRecorder::create();
 *   recorder->start(config);
 *   // ... recording ...
 *   auto result = recorder->stop();
 *   // recorder goes out of scope — RAII cleanup
 */
class Utilities_EXPORT IScreenRecorder
{
public:
    virtual ~IScreenRecorder() = default;

    /// Start recording with the given configuration.
    /// @return true if the recording started successfully.
    virtual bool start(const RecordingConfig& config) = 0;

    /// Stop the recording gracefully.
    /// @return Result with output path on success, or error message.
    virtual RecordingResult stop() = 0;

    /// Pause a running recording.
    virtual bool pause() = 0;

    /// Resume a paused recording.
    virtual bool resume() = 0;

    /// Whether this recorder has an active recording in progress.
    [[nodiscard]] virtual bool isActive() const = 0;

    /// The output file path (set during start).
    [[nodiscard]] virtual std::string outputPath() const = 0;

    /// Create a platform-specific recorder instance.
    static std::unique_ptr<IScreenRecorder> create();

    // === Static utilities (platform-dispatched) ===

    /// Auto-discover FFmpeg by locating the library and probing relative paths.
    static std::string findFFmpegPath();

    /// Search platform-specific candidate paths relative to appDir.
    static std::string findFFmpegPath(const std::string& appDir);

    /// Convenience wrapper around findFFmpegPath().
    static bool isFFmpegAvailable();

    /// Convenience wrapper around findFFmpegPath(appDir).
    static bool isFFmpegAvailable(const std::string& appDir);

    /// Check if the application has screen recording permission (macOS 10.15+).
    /// On Windows and Linux, always returns true.
    static bool hasScreenRecordingPermission();

    /// Check if the application has microphone permission (macOS 10.14+).
    /// On Windows and Linux, always returns true.
    static bool hasMicrophonePermission();

    /// Request microphone permission (macOS 10.14+).
    /// No-op on Windows and Linux (callback receives true immediately).
    static void requestMicrophonePermission(std::function<void(bool granted)> callback);

    /// Enumerate available audio devices (microphones, loopback sources, output devices).
    static std::vector<AudioDeviceInfo> enumerateAudioDevices();

    /// Convert a video file to GIF using FFmpeg.
    static bool convertToGif(const std::string& ffmpegPath,
                             const std::string& inputPath,
                             const std::string& outputPath,
                             int fps = 10);

    /// Extract a representative thumbnail frame from a video using FFmpeg.
    static bool extractThumbnail(const std::string& ffmpegPath,
                                 const std::string& inputPath,
                                 const std::string& outputPath,
                                 double timeSeconds = 0.2,
                                 int maxWidth = 320,
                                 int maxHeight = 180);
};

} // namespace ucf::utilities::screenrecording
