#pragma once

#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#ifdef _WIN32

#include <atomic>
#include <memory>
#include <string>

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>

// Forward declare to avoid pulling Windows.h into the header
struct WasapiLoopbackCapture;

namespace ucf::utilities::screenrecording {

/**
 * @brief Windows implementation of IScreenRecorder.
 *
 * Uses ProcessBridge for FFmpeg child process management, with optional
 * WASAPI loopback capture for system audio.
 */
class ScreenRecorder_Win final : public IScreenRecorder
{
public:
    ScreenRecorder_Win();
    ~ScreenRecorder_Win() override;

    // ── IScreenRecorder ──
    bool start(const RecordingConfig& config) override;
    RecordingResult stop() override;
    bool pause() override;
    bool resume() override;
    [[nodiscard]] bool isActive() const override;
    [[nodiscard]] std::string outputPath() const override;

    /// A DShow device entry with both friendly name (for UI) and moniker name (for FFmpeg).
    struct DShowDeviceEntry
    {
        std::string friendlyName;   ///< Human-readable (may contain non-ASCII)
        std::string monikerName;    ///< @device:cm:{...}\... — used by FFmpeg via MkParseDisplayName
    };

    // ── Static utilities (called from ScreenRecordingUtils) ──
    static std::string getLibraryDirectory();
    static std::string findFFmpegPath();
    static std::string findFFmpegPath(const std::string& appDir);
    static std::vector<DShowDeviceEntry> enumerateDShowCaptureDevices();
    static std::vector<AudioDeviceInfo> enumerateAudioDevices();
    static bool convertToGif(const std::string& ffmpegPath,
                             const std::string& inputPath,
                             const std::string& outputPath,
                             int fps);

private:
    std::atomic<bool> m_active{false};
    std::string m_outputPath;

    // FFmpeg child process (managed by ProcessBridge)
    std::unique_ptr<ucf::utilities::IProcessBridge> m_process;

    // WASAPI loopback capture (owned, heap-allocated to keep Windows API out of header)
    std::unique_ptr<WasapiLoopbackCapture> m_loopbackCapture;
    intptr_t m_hLoopbackRead = 0;   // named pipe handle for loopback audio
};

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
