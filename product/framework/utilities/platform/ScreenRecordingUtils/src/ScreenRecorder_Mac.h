#pragma once

#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#ifdef __APPLE__

#include <atomic>
#include <memory>
#include <string>

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>

namespace ucf::utilities::screenrecording {

/**
 * @brief macOS implementation of IScreenRecorder.
 *
 * Owns the FFmpeg child process via ProcessBridge. All resources are RAII-managed.
 */
class ScreenRecorder_Mac final : public IScreenRecorder
{
public:
    ScreenRecorder_Mac();
    ~ScreenRecorder_Mac() override;

    // ── IScreenRecorder ──
    bool start(const RecordingConfig& config) override;
    RecordingResult stop() override;
    bool pause() override;
    bool resume() override;
    [[nodiscard]] bool isActive() const override;
    [[nodiscard]] std::string outputPath() const override;

    // ── Static utilities (called from ScreenRecordingUtils) ──
    static std::string findFFmpegPath(const std::string& appDir);
    static bool hasScreenRecordingPermission();
    static bool hasMicrophonePermission();
    static void requestMicrophonePermission(std::function<void(bool)> callback);
    static std::vector<AudioDeviceInfo> enumerateAudioDevices();
    static bool convertToGif(const std::string& ffmpegPath,
                             const std::string& inputPath,
                             const std::string& outputPath,
                             int fps);

private:
    std::atomic<bool> m_active{false};
    std::string m_outputPath;
    std::unique_ptr<ucf::utilities::IProcessBridge> m_process;
};

} // namespace ucf::utilities::screenrecording

#endif // __APPLE__
