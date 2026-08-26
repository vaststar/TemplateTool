#include "VideoFrame.h"

#include <limits>

namespace ucf::service::media {

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

VideoFrame::VideoFrame(std::vector<uint8_t> data, int width, int height,
                       int bytesPerLine, PixelFormat format)
    : mData(std::move(data))
    , mWidth(width)
    , mHeight(height)
    , mBytesPerLine(bytesPerLine)
    , mFormat(format)
{
}

const uint8_t* VideoFrame::getData() const
{
    return mData.data();
}

size_t VideoFrame::getDataSize() const
{
    return mData.size();
}

int VideoFrame::getWidth() const
{
    return mWidth;
}

int VideoFrame::getHeight() const
{
    return mHeight;
}

int VideoFrame::getBytesPerLine() const
{
    return mBytesPerLine;
}

PixelFormat VideoFrame::getFormat() const
{
    return mFormat;
}

int64_t VideoFrame::getTimestampMs() const
{
    return mTimestampMs;
}

uint32_t VideoFrame::getFrameIndex() const
{
    return mFrameIndex;
}

bool VideoFrame::isValid() const
{
    if (mData.empty() || mWidth <= 0 || mHeight <= 0 || mBytesPerLine <= 0)
    {
        return false;
    }

    const auto pixelSize = bytesPerPixel(mFormat);
    if (pixelSize == 0)
    {
        return false;
    }

    const auto frameWidth = static_cast<std::size_t>(mWidth);
    if (frameWidth > std::numeric_limits<std::size_t>::max() / pixelSize)
    {
        return false;
    }

    const auto minimumRowSize = frameWidth * pixelSize;
    const auto stride = static_cast<std::size_t>(mBytesPerLine);
    if (stride < minimumRowSize)
    {
        return false;
    }

    const auto frameHeight = static_cast<std::size_t>(mHeight);
    if (frameHeight > std::numeric_limits<std::size_t>::max() / stride)
    {
        return false;
    }

    return mData.size() >= stride * frameHeight;
}

void VideoFrame::setTimestamp(int64_t ts)
{
    mTimestampMs = ts;
}

void VideoFrame::setFrameIndex(uint32_t idx)
{
    mFrameIndex = idx;
}

}  // namespace ucf::service::media
