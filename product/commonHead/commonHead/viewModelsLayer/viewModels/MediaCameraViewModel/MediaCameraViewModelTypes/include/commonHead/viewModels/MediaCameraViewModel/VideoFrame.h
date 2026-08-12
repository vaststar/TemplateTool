#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include <commonHead/viewModels/MediaCameraViewModel/MediaCameraViewModelTypesExport.h>

namespace commonHead::viewModels::model {

using ImageBuffer = std::vector<unsigned char>;

enum class PixelFormat
{
    Unknown,
    RGB888,
    RGBA8888,
    BGR888,
    BGRA8888
};

struct MEDIA_CAMERA_VIEW_MODEL_TYPES_API VideoFrame
{
    ImageBuffer data;
    int width = 0;
    int height = 0;
    int bytesPerLine = 0;
    PixelFormat format = PixelFormat::RGB888;

    VideoFrame() = default;

    VideoFrame(ImageBuffer buffer,
               int frameWidth,
               int frameHeight,
               int stride,
               PixelFormat pixelFormat = PixelFormat::RGB888)
        : data(std::move(buffer))
        , width(frameWidth)
        , height(frameHeight)
        , bytesPerLine(stride)
        , format(pixelFormat)
    {
    }

    [[nodiscard]] bool isValid() const
    {
        return !data.empty() && width > 0 && height > 0;
    }
};

} // namespace commonHead::viewModels::model
