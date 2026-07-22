#include "Resampler.h"

namespace ucf::utilities::ffmpeg::detail {

namespace {

AVSampleFormat toAVSampleFormat(SampleFormat format)
{
    switch (format)
    {
    case SampleFormat::S16: return AV_SAMPLE_FMT_S16;
    case SampleFormat::S32:  return AV_SAMPLE_FMT_S32;
    case SampleFormat::FLT:  return AV_SAMPLE_FMT_FLT;
    case SampleFormat::DBL:  return AV_SAMPLE_FMT_DBL;
    }
    return AV_SAMPLE_FMT_S16;
}

SampleFormat toPublicSampleFormat(AVSampleFormat format)
{
    switch (format)
    {
    case AV_SAMPLE_FMT_S16:
    case AV_SAMPLE_FMT_S16P: return SampleFormat::S16;
    case AV_SAMPLE_FMT_S32:
    case AV_SAMPLE_FMT_S32P: return SampleFormat::S32;
    case AV_SAMPLE_FMT_FLT:
    case AV_SAMPLE_FMT_FLTP: return SampleFormat::FLT;
    case AV_SAMPLE_FMT_DBL:
    case AV_SAMPLE_FMT_DBLP: return SampleFormat::DBL;
    default:                 return SampleFormat::S16;
    }
}

} // namespace

std::unique_ptr<Resampler> Resampler::create(const AVChannelLayout& srcLayout,
                                             AVSampleFormat srcFmt,
                                             int srcRate,
                                             SampleFormat dstFmt,
                                             int dstRate,
                                             int dstChannels)
{
    if (srcLayout.nb_channels <= 0 || srcRate <= 0)
    {
        FFL_LOG_ERROR("Resampler: invalid source layout/rate");
        return nullptr;
    }
    if (dstRate <= 0)
    {
        dstRate = srcRate;
    }
    if (dstChannels <= 0)
    {
        dstChannels = srcLayout.nb_channels;
    }

    AVChannelLayout dstLayout{};
    av_channel_layout_default(&dstLayout, dstChannels);

    SwrContext* raw = nullptr;
    int ret = swr_alloc_set_opts2(&raw, &dstLayout, toAVSampleFormat(dstFmt), dstRate,
                                  &srcLayout, srcFmt, srcRate,
                                  0, nullptr);
    if (ret < 0)
    {
        FFL_LOG_ERROR("Resampler: cannot create swresample context (" << errorString(ret)
                      << ")");
        av_channel_layout_uninit(&dstLayout);
        return nullptr;
    }
    SwrPtr swr(raw);
    ret = swr_init(swr.get());
    if (ret < 0)
    {
        FFL_LOG_ERROR("Resampler: cannot init swresample context (" << errorString(ret)
                      << ")");
        av_channel_layout_uninit(&dstLayout);
        return nullptr;
    }

    std::unique_ptr<Resampler> resampler(new Resampler());
    resampler->mSwr = std::move(swr);
    resampler->mDstRate = dstRate;
    resampler->mDstFmt = toAVSampleFormat(dstFmt);
    resampler->mDstChannels = dstChannels;
    resampler->mPublicFmt = dstFmt;
    av_channel_layout_uninit(&dstLayout);
    return resampler;
}

bool Resampler::convert(const AVFrame* frame, RawAudio& out) const
{
    if (frame == nullptr)
    {
        FFL_LOG_ERROR("Resampler: null frame");
        return false;
    }

    const int srcRate = frame->sample_rate > 0 ? frame->sample_rate : mDstRate;
    const int outSamples = av_rescale_rnd(swr_get_delay(mSwr.get(), srcRate) + frame->nb_samples,
                                          mDstRate, srcRate, AV_ROUND_UP);
    if (outSamples <= 0)
    {
        FFL_LOG_ERROR("Resampler: invalid output sample count");
        return false;
    }

    const int bufferSize = av_samples_get_buffer_size(nullptr, mDstChannels, outSamples,
                                                      mDstFmt, 1);
    if (bufferSize < 0)
    {
        FFL_LOG_ERROR("Resampler: cannot size destination buffer (" << errorString(bufferSize)
                      << ")");
        return false;
    }

    out.samples.resize(static_cast<size_t>(bufferSize));
    uint8_t* dstData[1] = {out.samples.data()};
    const uint8_t** srcData = const_cast<const uint8_t**>(frame->extended_data);
    const int converted = swr_convert(mSwr.get(), dstData, outSamples,
                                      srcData, frame->nb_samples);
    if (converted < 0)
    {
        FFL_LOG_ERROR("Resampler: convert failed (" << errorString(converted) << ")");
        return false;
    }

    const int actualSize = av_samples_get_buffer_size(nullptr, mDstChannels, converted,
                                                      mDstFmt, 1);
    if (actualSize < 0)
    {
        FFL_LOG_ERROR("Resampler: cannot finalize destination buffer ("
                      << errorString(actualSize) << ")");
        return false;
    }

    out.samples.resize(static_cast<size_t>(actualSize));
    out.sampleRate = mDstRate;
    out.channels = mDstChannels;
    out.format = mPublicFmt;
    return true;
}

} // namespace ucf::utilities::ffmpeg::detail
