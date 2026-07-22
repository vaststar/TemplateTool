#include "Scaler.h"

namespace ucf::utilities::ffmpeg::detail {

namespace {

AVPixelFormat toAVPixelFormat(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::BGRA:  return AV_PIX_FMT_BGRA;
    case PixelFormat::RGBA:  return AV_PIX_FMT_RGBA;
    case PixelFormat::RGB24: return AV_PIX_FMT_RGB24;
    }
    return AV_PIX_FMT_BGRA;
}

} // namespace

std::unique_ptr<Scaler> Scaler::create(int srcW, int srcH, AVPixelFormat srcFmt,
                                       int dstW, int dstH, PixelFormat dstFmt)
{
    if (srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0)
    {
        FFL_LOG_ERROR("Scaler: invalid geometry " << srcW << "x" << srcH
                      << " -> " << dstW << "x" << dstH);
        return nullptr;
    }

    SwsPtr sws(sws_getContext(
        srcW, srcH, srcFmt,
        dstW, dstH, toAVPixelFormat(dstFmt),
        SWS_BILINEAR, nullptr, nullptr, nullptr));
    if (!sws)
    {
        FFL_LOG_ERROR("Scaler: cannot create swscale context");
        return nullptr;
    }

    std::unique_ptr<Scaler> scaler(new Scaler());
    scaler->mSws = std::move(sws);
    scaler->mDstW = dstW;
    scaler->mDstH = dstH;
    scaler->mDstFmt = dstFmt;
    scaler->mBytesPerPixel = bytesPerPixel(dstFmt);
    return scaler;
}

bool Scaler::convert(const AVFrame* frame, RawImage& out) const
{
    if (frame == nullptr)
    {
        FFL_LOG_ERROR("Scaler: null frame");
        return false;
    }

    out.width = mDstW;
    out.height = mDstH;
    out.format = mDstFmt;
    out.pixels.resize(static_cast<size_t>(mDstW) * static_cast<size_t>(mDstH)
                      * static_cast<size_t>(mBytesPerPixel));

    uint8_t* dstData[4] = {out.pixels.data(), nullptr, nullptr, nullptr};
    int dstLinesize[4] = {mDstW * mBytesPerPixel, 0, 0, 0};

    sws_scale(mSws.get(), frame->data, frame->linesize, 0, frame->height, dstData, dstLinesize);
    return true;
}

} // namespace ucf::utilities::ffmpeg::detail
