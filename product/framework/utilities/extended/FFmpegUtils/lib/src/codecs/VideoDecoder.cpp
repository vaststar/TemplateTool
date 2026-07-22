#include "VideoDecoder.h"

namespace ucf::utilities::ffmpeg::detail {

std::unique_ptr<VideoDecoder> VideoDecoder::create(const AVCodec* decoder,
                                                   const AVCodecParameters* par)
{
    if (decoder == nullptr || par == nullptr)
    {
        FFL_LOG_ERROR("VideoDecoder: null decoder/params");
        return nullptr;
    }

    CodecCtxPtr ctx(avcodec_alloc_context3(decoder));
    if (!ctx)
    {
        FFL_LOG_ERROR("VideoDecoder: cannot alloc codec context");
        return nullptr;
    }
    if (int ret = avcodec_parameters_to_context(ctx.get(), par); ret < 0)
    {
        FFL_LOG_ERROR("VideoDecoder: cannot copy codec params (" << errorString(ret) << ")");
        return nullptr;
    }
    if (int ret = avcodec_open2(ctx.get(), decoder, nullptr); ret < 0)
    {
        FFL_LOG_ERROR("VideoDecoder: cannot open decoder (" << errorString(ret) << ")");
        return nullptr;
    }

    std::unique_ptr<VideoDecoder> dec(new VideoDecoder());
    dec->mCtx = std::move(ctx);
    return dec;
}

} // namespace ucf::utilities::ffmpeg::detail
