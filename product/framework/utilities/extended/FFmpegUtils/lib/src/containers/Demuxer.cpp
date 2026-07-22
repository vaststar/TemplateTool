#include "Demuxer.h"

namespace ucf::utilities::ffmpeg::detail {

namespace {

MediaInfo buildInfo(AVFormatContext* fmt)
{
    MediaInfo info;
    info.formatName = fmt->iformat && fmt->iformat->name ? fmt->iformat->name : "";
    if (fmt->duration != AV_NOPTS_VALUE && fmt->duration > 0)
    {
        info.durationMs = av_rescale(fmt->duration, 1000, AV_TIME_BASE);
    }
    info.bitrate = fmt->bit_rate > 0 ? fmt->bit_rate : 0;

    info.streams.reserve(fmt->nb_streams);
    for (unsigned i = 0; i < fmt->nb_streams; ++i)
    {
        AVStream* stream = fmt->streams[i];
        const AVCodecParameters* par = stream->codecpar;

        StreamInfo s;
        switch (par->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:    s.type = MediaType::Video; break;
        case AVMEDIA_TYPE_AUDIO:    s.type = MediaType::Audio; break;
        case AVMEDIA_TYPE_SUBTITLE: s.type = MediaType::Subtitle; break;
        default:                    s.type = MediaType::Unknown; break;
        }

        const char* codecName = avcodec_get_name(par->codec_id);
        s.codec = codecName ? codecName : "";
        s.bitrate = par->bit_rate > 0 ? par->bit_rate : 0;

        if (s.type == MediaType::Video)
        {
            s.width = par->width;
            s.height = par->height;
            AVRational fr = av_guess_frame_rate(fmt, stream, nullptr);
            s.fps = (fr.num > 0 && fr.den > 0) ? av_q2d(fr) : 0.0;
        }
        else if (s.type == MediaType::Audio)
        {
            s.sampleRate = par->sample_rate;
            s.channels = par->ch_layout.nb_channels;
        }

        info.streams.push_back(std::move(s));
    }
    return info;
}

} // namespace

std::unique_ptr<Demuxer> Demuxer::open(const std::string& url)
{
    installLogBridge();

    if (url.empty())
    {
        FFL_LOG_ERROR("Demuxer: empty url");
        return nullptr;
    }

    AVFormatContext* rawFmt = nullptr;
    if (int ret = avformat_open_input(&rawFmt, url.c_str(), nullptr, nullptr); ret != 0)
    {
        FFL_LOG_ERROR("Demuxer: cannot open input: " << url << " (" << errorString(ret) << ")");
        return nullptr;
    }
    FormatCtxPtr fmt(rawFmt);

    if (int ret = avformat_find_stream_info(fmt.get(), nullptr); ret < 0)
    {
        FFL_LOG_ERROR("Demuxer: cannot find stream info: " << url << " (" << errorString(ret) << ")");
        return nullptr;
    }

    std::unique_ptr<Demuxer> demuxer(new Demuxer());
    demuxer->mInfo = buildInfo(fmt.get());
    demuxer->mFormat = std::move(fmt);
    return demuxer;
}

bool Demuxer::seekToMs(int streamIndex, std::int64_t timestampMs) const
{
    if (!mFormat || streamIndex < 0 || timestampMs < 0)
    {
        FFL_LOG_ERROR("Demuxer: invalid seek request");
        return false;
    }

    AVStream* stream = mFormat->streams[streamIndex];
    const std::int64_t target = av_rescale_q(timestampMs, AVRational{1, 1000}, stream->time_base);
    if (int ret = av_seek_frame(mFormat.get(), streamIndex, target,
                                AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY); ret < 0)
    {
        FFL_LOG_ERROR("Demuxer: seek failed: " << errorString(ret));
        return false;
    }
    return true;
}

int Demuxer::bestVideoStream(const AVCodec** outDecoder) const
{
    return av_find_best_stream(mFormat.get(), AVMEDIA_TYPE_VIDEO, -1, -1, outDecoder, 0);
}

} // namespace ucf::utilities::ffmpeg::detail
