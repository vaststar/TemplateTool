#pragma once

#include <memory>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegTypes.h>

#include "../FFmpegInternal.h"

namespace ucf::utilities::ffmpeg::detail {

// Low-level pixel converter: wraps a SwsContext to turn decoded frames (any
// pixel format) into a tightly packed, top-down buffer of a chosen packed
// PixelFormat. Private module (libav types allowed). One scaler is bound to a
// fixed src/dst geometry + source format + destination format.
class Scaler
{
public:
    // Creates a converter from (srcW, srcH, srcFmt) to (dstW, dstH, dstFmt).
    // dstFmt defaults to BGRA (desktop display). Returns nullptr on failure.
    static std::unique_ptr<Scaler> create(int srcW, int srcH, AVPixelFormat srcFmt,
                                          int dstW, int dstH,
                                          PixelFormat dstFmt = PixelFormat::BGRA);

    Scaler(const Scaler&) = delete;
    Scaler& operator=(const Scaler&) = delete;

    // Converts @p frame into a tightly packed, top-down RawImage tagged with the
    // destination format. Returns false on failure.
    bool convert(const AVFrame* frame, RawImage& out) const;

private:
    Scaler() = default;

    SwsPtr mSws;
    int mDstW = 0;
    int mDstH = 0;
    PixelFormat mDstFmt = PixelFormat::BGRA;
    int mBytesPerPixel = 4;
};

} // namespace ucf::utilities::ffmpeg::detail
