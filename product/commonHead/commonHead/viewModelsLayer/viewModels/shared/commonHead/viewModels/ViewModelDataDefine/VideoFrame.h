#pragma once

#include <vector>
#include <cstdint>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::viewModels::model{

using ImageBuffer = std::vector<unsigned char>;

enum class PixelFormat {
    Unknown,
    RGB888,
    RGBA8888,
    BGR888,
    BGRA8888
};

struct COMMONHEAD_EXPORT VideoFrame
{
    ImageBuffer data;
    int width = 0;
    int height = 0;
    int bytesPerLine = 0;
    PixelFormat format = PixelFormat::RGB888;

    VideoFrame() = default;
    
    VideoFrame(ImageBuffer buf, int w, int h, int stride, 
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
}