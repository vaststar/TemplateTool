#pragma once

#include <cstdint>
#include <memory>

namespace ucf::service::media {

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
