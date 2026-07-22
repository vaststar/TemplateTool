#pragma once

#include <memory>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegTypes.h>

#include "../FFmpegInternal.h"

namespace ucf::utilities::ffmpeg::detail {

// Low-level audio converter: wraps a SwrContext to turn decoded frames into a
// tightly packed interleaved PCM buffer of a chosen SampleFormat. Private
// module (libav types allowed). One resampler is bound to a fixed src/dst
// layout, sample format and sample rate.
class Resampler
{
public:
    // Creates a converter from source to destination layout / format / rate.
    // Returns nullptr on failure.
    static std::unique_ptr<Resampler> create(const AVChannelLayout& srcLayout,
                                             AVSampleFormat srcFmt,
                                             int srcRate,
                                             SampleFormat dstFmt = SampleFormat::S16,
                                             int dstRate = 0,
                                             int dstChannels = 0);

    Resampler(const Resampler&) = delete;
    Resampler& operator=(const Resampler&) = delete;

    // Converts @p frame into a tightly packed, interleaved RawAudio tagged with
    // the destination format. Returns false on failure.
    bool convert(const AVFrame* frame, RawAudio& out) const;

private:
    Resampler() = default;

    SwrPtr mSwr;
    int mDstRate = 0;
    AVSampleFormat mDstFmt = AV_SAMPLE_FMT_S16;
    int mDstChannels = 0;
    SampleFormat mPublicFmt = SampleFormat::S16;
};

} // namespace ucf::utilities::ffmpeg::detail
