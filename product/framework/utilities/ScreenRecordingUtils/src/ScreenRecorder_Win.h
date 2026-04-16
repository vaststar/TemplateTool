#pragma once

#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#ifdef _WIN32

#include <atomic>
#include <memory>
#include <string>

// Forward declare to avoid pulling Windows.h into the header
struct WasapiLoopbackCapture;

namespace ucf::utilities::screenrecording {

/**
 * @brief Windows implementation of IScreenRecorder.
 *
 * Owns the FFmpeg child process, stdin pipe, and optional WASAPI loopback
 * capture thread. All resources are RAII-managed.
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

    // ── Static utilities (called from ScreenRecordingUtils) ──
    static std::string findFFmpegPath(const std::string& appDir);
    static std::vector<AudioDeviceInfo> enumerateAudioDevices();
    static bool convertToGif(const std::string& ffmpegPath,
                             const std::string& inputPath,
                             const std::string& outputPath,
                             int fps);

private:
    std::atomic<bool> m_active{false};
    std::string m_outputPath;

    // FFmpeg child process handles (stored as intptr_t to avoid windows.h)
    intptr_t m_hProcess = 0;
    intptr_t m_hStdinWrite = 0;

    // WASAPI loopback capture (owned, heap-allocated to keep Windows API out of header)
    std::unique_ptr<WasapiLoopbackCapture> m_loopbackCapture;
    intptr_t m_hLoopbackRead = 0;   // read-end of loopback pipe (inherited by FFmpeg)
};

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
