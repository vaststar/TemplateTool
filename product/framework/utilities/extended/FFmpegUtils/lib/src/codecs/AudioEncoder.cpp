#include "AudioEncoder.h"

namespace ucf::utilities::ffmpeg::detail {

std::unique_ptr<AudioEncoder> AudioEncoder::create(const AVCodec* encoder,
                                                    const AVChannelLayout& layout,
                                                    int sampleRate,
                                                    AVSampleFormat sampleFormat,
                                                    int bitRate,
                                                    bool useGlobalHeader)
{
    if (encoder == nullptr || layout.nb_channels <= 0 || sampleRate <= 0)
    {
        FFL_LOG_ERROR("AudioEncoder: invalid encoder/layout/rate");
        return nullptr;
    }

    CodecCtxPtr ctx(avcodec_alloc_context3(encoder));
    if (!ctx)
    {
        FFL_LOG_ERROR("AudioEncoder: cannot alloc codec context");
        return nullptr;
    }

    ctx->sample_rate = sampleRate;
    ctx->sample_fmt = sampleFormat;
    if (bitRate > 0)
    {
        ctx->bit_rate = bitRate;
    }
    if (useGlobalHeader)
    {
        ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    if (int ret = av_channel_layout_copy(&ctx->ch_layout, &layout); ret < 0)
    {
        FFL_LOG_ERROR("AudioEncoder: cannot copy channel layout (" << errorString(ret) << ")");
        return nullptr;
    }

    if (int ret = avcodec_open2(ctx.get(), encoder, nullptr); ret < 0)
    {
        FFL_LOG_ERROR("AudioEncoder: cannot open encoder (" << errorString(ret) << ")");
        return nullptr;
    }

    std::unique_ptr<AudioEncoder> enc(new AudioEncoder());
    enc->mCtx = std::move(ctx);
    return enc;
}

} // namespace ucf::utilities::ffmpeg::detail
