#include "VideoFrame.h"

namespace ucf::service::media {

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
    return !mData.empty() && mWidth > 0 && mHeight > 0;
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
