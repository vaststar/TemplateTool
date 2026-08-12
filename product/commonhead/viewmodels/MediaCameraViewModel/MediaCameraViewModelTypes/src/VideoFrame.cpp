#include <commonhead/viewmodels/MediaCameraViewModel/VideoFrame.h>

#include <utility>

namespace commonHead::viewModels::model {

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
    return !data.empty() && width > 0 && height > 0;
}

} // namespace commonHead::viewModels::model
