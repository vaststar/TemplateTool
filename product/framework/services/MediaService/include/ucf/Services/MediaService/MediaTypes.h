#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <variant>

namespace ucf::service::media {

// 本地摄像头：通过 OS 设备索引打开
struct LocalCameraSource
{
    int index = 0;

    std::string toKey() const { return "local:" + std::to_string(index); }
};

// 网络流摄像头：rtsp:// / http:// / https:// / file:// 等 OpenCV/FFmpeg 支持的 URL
struct NetworkCameraSource
{
    std::string url;
    std::string transport;     // "tcp" / "udp"，留空使用默认
    int openTimeoutMs = 5000;  // 打开超时
    int readTimeoutMs = 5000;  // 读帧超时

    std::string toKey() const { return "url:" + url; }
};

// 封闭集多态：后续可扩展 FileSource / VirtualSource 等
using CameraSource = std::variant<LocalCameraSource, NetworkCameraSource>;

// 去重 key：用于 CameraManager 内部识别“同一个源”
inline std::string toKey(const CameraSource& source)
{
    return std::visit([](const auto& s) { return s.toKey(); }, source);
}

enum class PixelFormat
{
    Unknown,
    RGB888,       // 3 bytes per pixel
    RGBA8888,     // 4 bytes per pixel
    BGR888,       // 3 bytes per pixel
    BGRA8888      // 4 bytes per pixel
};

// 纯虚接口 - 只有 getter
class IVideoFrame
{
public:
    virtual ~IVideoFrame() = default;

    [[nodiscard]] virtual const uint8_t* getData() const = 0;
    [[nodiscard]] virtual size_t getDataSize() const = 0;
    [[nodiscard]] virtual int getWidth() const = 0;
    [[nodiscard]] virtual int getHeight() const = 0;
    [[nodiscard]] virtual int getBytesPerLine() const = 0;
    [[nodiscard]] virtual PixelFormat getFormat() const = 0;
    [[nodiscard]] virtual int64_t getTimestampMs() const = 0;
    [[nodiscard]] virtual uint32_t getFrameIndex() const = 0;
    [[nodiscard]] virtual bool isValid() const = 0;
};

using IVideoFramePtr = std::shared_ptr<IVideoFrame>;

}  // namespace ucf::service::media
