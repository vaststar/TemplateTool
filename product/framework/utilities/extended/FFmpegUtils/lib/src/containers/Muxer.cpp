#include "Muxer.h"

namespace ucf::utilities::ffmpeg::detail {

std::unique_ptr<Muxer> Muxer::create(const std::string& path, const std::string& formatName)
{
    if (path.empty())
    {
        FFL_LOG_ERROR("Muxer: empty output path");
        return nullptr;
    }

    AVFormatContext* raw = nullptr;
    const char* fmtName = formatName.empty() ? nullptr : formatName.c_str();
    if (int ret = avformat_alloc_output_context2(&raw, nullptr, fmtName, path.c_str()); ret < 0 || raw == nullptr)
    {
        FFL_LOG_ERROR("Muxer: cannot create output context (" << errorString(ret) << ")");
        return nullptr;
    }

    std::unique_ptr<Muxer> muxer(new Muxer());
    muxer->mFormat = FormatCtxPtr(raw);
    muxer->mPath = path;
    return muxer;
}

int Muxer::addStream(const AVCodecContext* codecCtx)
{
    if (mFormat == nullptr || codecCtx == nullptr)
    {
        return AVERROR(EINVAL);
    }

    AVStream* stream = avformat_new_stream(mFormat.get(), nullptr);
    if (!stream)
    {
        return AVERROR(ENOMEM);
    }
    if (int ret = avcodec_parameters_from_context(stream->codecpar, codecCtx); ret < 0)
    {
        return ret;
    }
    stream->time_base = codecCtx->time_base;
    return 0;
}

int Muxer::writeHeader()
{
    if (mFormat == nullptr)
    {
        return AVERROR(EINVAL);
    }

    if (!(mFormat->oformat->flags & AVFMT_NOFILE))
    {
        if (int ret = avio_open(&mFormat->pb, mPath.c_str(), AVIO_FLAG_WRITE); ret < 0)
        {
            return ret;
        }
    }
    return avformat_write_header(mFormat.get(), nullptr);
}

int Muxer::writePacket(const AVPacket* packet)
{
    if (mFormat == nullptr || packet == nullptr)
    {
        return AVERROR(EINVAL);
    }
    return av_interleaved_write_frame(mFormat.get(), const_cast<AVPacket*>(packet));
}

int Muxer::writeTrailer()
{
    if (mFormat == nullptr)
    {
        return AVERROR(EINVAL);
    }

    const int ret = av_write_trailer(mFormat.get());
    if (!(mFormat->oformat->flags & AVFMT_NOFILE) && mFormat->pb)
    {
        avio_closep(&mFormat->pb);
    }
    return ret;
}

} // namespace ucf::utilities::ffmpeg::detail
