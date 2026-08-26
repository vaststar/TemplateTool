#include <commonhead/viewmodels/MediaCameraViewModel/VideoFrame.h>

#include <limits>
#include <utility>

namespace commonHead::viewModels::model {

namespace {

std::size_t bytesPerPixel(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::RGB888:
    case PixelFormat::BGR888:
        return 3;
    case PixelFormat::RGBA8888:
    case PixelFormat::BGRA8888:
        return 4;
    case PixelFormat::Unknown:
        return 0;
    }
    return 0;
}

} // namespace

VideoFrame::VideoFrame() = default;

VideoFrame::VideoFrame(ImageBuffer buffer,
                       int frameWidth,
                       int frameHeight,
                       int stride,
                       PixelFormat pixelFormat)
    : data(std::move(buffer))
    , width(frameWidth)
    , height(frameHeight)
    , bytesPerLine(stride)
    , format(pixelFormat)
{
}

VideoFrame::VideoFrame(const VideoFrame&) = default;
VideoFrame::VideoFrame(VideoFrame&&) noexcept = default;
VideoFrame& VideoFrame::operator=(const VideoFrame&) = default;
VideoFrame& VideoFrame::operator=(VideoFrame&&) noexcept = default;
VideoFrame::~VideoFrame() = default;

bool VideoFrame::isValid() const
{
    if (data.empty() || width <= 0 || height <= 0 || bytesPerLine <= 0)
    {
        return false;
    }

    const auto pixelSize = bytesPerPixel(format);
    if (pixelSize == 0)
    {
        return false;
    }

    const auto frameWidth = static_cast<std::size_t>(width);
    if (frameWidth > std::numeric_limits<std::size_t>::max() / pixelSize)
    {
        return false;
    }

    const auto minimumRowSize = frameWidth * pixelSize;
    const auto stride = static_cast<std::size_t>(bytesPerLine);
    if (stride < minimumRowSize)
    {
        return false;
    }

    const auto frameHeight = static_cast<std::size_t>(height);
    if (frameHeight > std::numeric_limits<std::size_t>::max() / stride)
    {
        return false;
    }

    return data.size() >= stride * frameHeight;
}

} // namespace commonHead::viewModels::model
