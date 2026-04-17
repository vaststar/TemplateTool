#pragma once

#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#ifdef __linux__

#include <atomic>
#include <cstdint>
#include <string>

namespace ucf::utilities::screenrecording {

/**
 * @brief Linux implementation of IScreenRecorder.
 *
 * Owns the recording child process (Python portal recorder on Wayland)
 * and all associated resources. All resources are RAII-managed.
 */
class ScreenRecorder_Linux final : public IScreenRecorder
{
public:
    ScreenRecorder_Linux();
    ~ScreenRecorder_Linux() override;

    // ── IScreenRecorder ──
    bool start(const RecordingConfig& config) override;
    RecordingResult stop() override;
    bool pause() override;
    bool resume() override;
    [[nodiscard]] bool isActive() const override;
    [[nodiscard]] std::string outputPath() const override;

    // ── Static utilities (called from ScreenRecordingUtils) ──
    static std::string getLibraryDirectory();
    static std::string findFFmpegPath();
    static std::string findFFmpegPath(const std::string& appDir);
    static std::vector<AudioDeviceInfo> enumerateAudioDevices();
    static bool convertToGif(const std::string& ffmpegPath,
                             const std::string& inputPath,
                             const std::string& outputPath,
                             int fps);

private:
    std::atomic<bool> m_active{false};
    std::string m_outputPath;
    int64_t m_pid = -1;
    int m_stdoutPipeFd = -1;   // read-end of child's stdout (protocol messages)

    // Wayland portal recording state
    std::string m_waylandTempPath;
    std::string m_ffmpegPath;

    // Region crop (applied via FFmpeg after recording stops)
    bool m_isRegion = false;
    int m_regionX = 0, m_regionY = 0;
    int m_regionW = 0, m_regionH = 0;
};

} // namespace ucf::utilities::screenrecording

#endif // __linux__
