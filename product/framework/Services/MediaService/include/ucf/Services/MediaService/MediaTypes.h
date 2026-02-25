#pragma once

#include <cstdint>
#include <vector>

namespace ucf::service::media {

enum class PixelFormat {
    Unknown,
    RGB888,       // 3 bytes per pixel
    RGBA8888,     // 4 bytes per pixel
    BGR888,       // 3 bytes per pixel
    BGRA8888      // 4 bytes per pixel
};

struct VideoFrame {
    std::vector<uint8_t> data;
    int width = 0;
    int height = 0;
    int bytesPerLine = 0;
    PixelFormat format = PixelFormat::RGB888;
    int64_t timestampMs = 0;
    uint32_t frameIndex = 0;

    VideoFrame() = default;
    
    VideoFrame(std::vector<uint8_t> buf, int w, int h, int stride, 
               PixelFormat fmt = PixelFormat::RGB888)
        : data(std::move(buf))
        , width(w)
        , height(h)
        , bytesPerLine(stride)
        , format(fmt)
    {}

    [[nodiscard]] bool isValid() const {
        return !data.empty() && width > 0 && height > 0;
    }
};

}  // namespace ucf::service::media
