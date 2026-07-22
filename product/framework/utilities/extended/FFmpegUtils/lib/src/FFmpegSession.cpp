#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegSession.h>

#include "codecs/AudioDecoder.h"
#include "codecs/AudioEncoder.h"
#include "containers/Demuxer.h"
#include "FFmpegInternal.h"
#include "containers/Muxer.h"
#include "filters/Resampler.h"
#include "filters/Scaler.h"
#include "codecs/VideoDecoder.h"

namespace ucf::utilities::ffmpeg {

using namespace detail;

// ============================================================================
// Helper functions (moved from FFmpegLib.cpp)
// ============================================================================

namespace {

bool convertFrameToRawImage(const AVFrame* frame, int width, int height, RawImage& out)
{
    auto scaler = Scaler::create(width, height, static_cast<AVPixelFormat>(frame->format),
                                 width, height, PixelFormat::BGRA);
    if (!scaler)
    {
        return false;
    }
    return scaler->convert(frame, out);
}

bool convertFrameToRawAudio(const AVFrame* frame,
                            AudioDecoder& audioDecoder,
                            const AudioOutputOptions& output,
                            RawAudio& out)
{
    AVChannelLayout srcLayout{};
    if (audioDecoder.channelLayout() != nullptr && audioDecoder.channelLayout()->nb_channels > 0)
    {
        if (int ret = av_channel_layout_copy(&srcLayout, audioDecoder.channelLayout()); ret < 0)
        {
            FFL_LOG_ERROR("convertFrameToRawAudio: cannot copy channel layout ("
                          << errorString(ret) << ")");
            return false;
        }
    }
    else if (audioDecoder.channels() > 0)
    {
        av_channel_layout_default(&srcLayout, audioDecoder.channels());
    }
    else
    {
        FFL_LOG_ERROR("convertFrameToRawAudio: invalid channel layout");
        return false;
    }

    const int dstRate = output.sampleRate > 0 ? output.sampleRate : audioDecoder.sampleRate();
    const int dstChannels = output.channels > 0 ? output.channels : audioDecoder.channels();
    auto resampler = Resampler::create(srcLayout,
                                       audioDecoder.sampleFormat(),
                                       audioDecoder.sampleRate(),
                                       output.format,
                                       dstRate,
                                       dstChannels);
    av_channel_layout_uninit(&srcLayout);
    if (!resampler)
    {
        return false;
    }
    return resampler->convert(frame, out);
}

bool receiveFrameAtOrAfterTimestamp(VideoDecoder& videoDecoder,
                                    FramePtr& frame,
                                    const AVRational& streamTimeBase,
                                    std::optional<std::int64_t> minTimestampMs)
{
    while (true)
    {
        int ret = videoDecoder.receive(frame.get());
        if (ret == 0)
        {
            if (minTimestampMs.has_value())
            {
                const std::int64_t frameTimestampMs = frame->best_effort_timestamp == AV_NOPTS_VALUE
                    ? AV_NOPTS_VALUE
                    : av_rescale_q(frame->best_effort_timestamp, streamTimeBase, AVRational{1, 1000});
                if (frameTimestampMs != AV_NOPTS_VALUE && frameTimestampMs < *minTimestampMs)
                {
                    av_frame_unref(frame.get());
                    continue;
                }
            }
            return true;
        }
        if (ret != AVERROR(EAGAIN))
        {
            FFL_LOG_ERROR("receiveFrameAtOrAfterTimestamp[video]: decode error: " << errorString(ret));
            return false;
        }
        return false;
    }
}

bool receiveFrameAtOrAfterTimestamp(AudioDecoder& audioDecoder,
                                    FramePtr& frame,
                                    const AVRational& streamTimeBase,
                                    std::optional<std::int64_t> minTimestampMs)
{
    while (true)
    {
        int ret = audioDecoder.receive(frame.get());
        if (ret == 0)
        {
            if (minTimestampMs.has_value())
            {
                const std::int64_t frameTimestampMs = frame->best_effort_timestamp == AV_NOPTS_VALUE
                    ? AV_NOPTS_VALUE
                    : av_rescale_q(frame->best_effort_timestamp, streamTimeBase, AVRational{1, 1000});
                if (frameTimestampMs != AV_NOPTS_VALUE && frameTimestampMs < *minTimestampMs)
                {
                    av_frame_unref(frame.get());
                    continue;
                }
            }
            return true;
        }
        if (ret != AVERROR(EAGAIN))
        {
            FFL_LOG_ERROR("receiveFrameAtOrAfterTimestamp[audio]: decode error: " << errorString(ret));
            return false;
        }
        return false;
    }
}

std::optional<RawImage> decodeBestVideoFrame(Demuxer& demuxer,
                                             int streamIndex,
                                             const AVCodec* decoder,
                                             std::optional<std::int64_t> minTimestampMs = std::nullopt)
{
    if (streamIndex < 0 || decoder == nullptr)
    {
        FFL_LOG_ERROR("decodeBestVideoFrame: invalid video stream");
        return std::nullopt;
    }

    AVFormatContext* fmt = demuxer.format();
    const AVRational streamTimeBase = fmt->streams[streamIndex]->time_base;
    auto videoDecoder = VideoDecoder::create(decoder, fmt->streams[streamIndex]->codecpar);
    if (!videoDecoder)
    {
        return std::nullopt;
    }

    PacketPtr packet(av_packet_alloc());
    FramePtr frame(av_frame_alloc());
    if (!packet || !frame)
    {
        FFL_LOG_ERROR("decodeBestVideoFrame: cannot alloc packet/frame");
        return std::nullopt;
    }

    bool haveFrame = false;
    while (!haveFrame && demuxer.readPacket(packet.get()) >= 0)
    {
        if (packet->stream_index == streamIndex)
        {
            if (videoDecoder->send(packet.get()) == 0)
            {
                haveFrame = receiveFrameAtOrAfterTimestamp(*videoDecoder, frame, streamTimeBase,
                                                           minTimestampMs);
                if (!haveFrame)
                {
                    av_packet_unref(packet.get());
                    return std::nullopt;
                }
            }
        }
        av_packet_unref(packet.get());
    }

    if (!haveFrame)
    {
        videoDecoder->send(nullptr);
        haveFrame = receiveFrameAtOrAfterTimestamp(*videoDecoder, frame, streamTimeBase,
                                                   minTimestampMs);
    }

    if (!haveFrame)
    {
        FFL_LOG_ERROR("decodeBestVideoFrame: no decodable frame");
        return std::nullopt;
    }

    const int width = videoDecoder->width();
    const int height = videoDecoder->height();
    if (width <= 0 || height <= 0)
    {
        FFL_LOG_ERROR("decodeBestVideoFrame: invalid dimensions " << width << "x" << height);
        return std::nullopt;
    }

    RawImage out;
    if (!convertFrameToRawImage(frame.get(), width, height, out))
    {
        return std::nullopt;
    }
    return out;
}

std::optional<RawAudio> decodeBestAudioFrame(Demuxer& demuxer,
                                             int streamIndex,
                                             const AVCodec* decoder,
                                             std::optional<std::int64_t> minTimestampMs,
                                             const AudioOutputOptions& output)
{
    if (streamIndex < 0 || decoder == nullptr)
    {
        FFL_LOG_ERROR("decodeBestAudioFrame: invalid audio stream");
        return std::nullopt;
    }

    AVFormatContext* fmt = demuxer.format();
    const AVRational streamTimeBase = fmt->streams[streamIndex]->time_base;
    auto audioDecoder = AudioDecoder::create(decoder, fmt->streams[streamIndex]->codecpar);
    if (!audioDecoder)
    {
        return std::nullopt;
    }

    PacketPtr packet(av_packet_alloc());
    FramePtr frame(av_frame_alloc());
    if (!packet || !frame)
    {
        FFL_LOG_ERROR("decodeBestAudioFrame: cannot alloc packet/frame");
        return std::nullopt;
    }

    bool haveFrame = false;
    while (!haveFrame && demuxer.readPacket(packet.get()) >= 0)
    {
        if (packet->stream_index == streamIndex)
        {
            if (audioDecoder->send(packet.get()) == 0)
            {
                haveFrame = receiveFrameAtOrAfterTimestamp(*audioDecoder, frame, streamTimeBase,
                                                           minTimestampMs);
                if (!haveFrame)
                {
                    av_packet_unref(packet.get());
                    return std::nullopt;
                }
            }
        }
        av_packet_unref(packet.get());
    }

    if (!haveFrame)
    {
        audioDecoder->send(nullptr);
        haveFrame = receiveFrameAtOrAfterTimestamp(*audioDecoder, frame, streamTimeBase,
                                                   minTimestampMs);
    }

    if (!haveFrame)
    {
        FFL_LOG_ERROR("decodeBestAudioFrame: no decodable frame");
        return std::nullopt;
    }

    RawAudio out;
    if (!convertFrameToRawAudio(frame.get(), *audioDecoder, output, out))
    {
        return std::nullopt;
    }
    return out;
}

} // namespace

// ============================================================================
// FFmpegSession::Impl - Private implementation holding the open context
// ============================================================================

class FFmpegSession::Impl
{
public:
    std::unique_ptr<Demuxer> demuxer;
    // Could add frame caches, decoder caches here for future optimization
};

// ============================================================================
// FFmpegSession - Public interface
// ============================================================================

FFmpegSession::FFmpegSession()
    : mImpl(std::make_unique<Impl>())
{
}

FFmpegSession::~FFmpegSession()
{
    close();
}

bool FFmpegSession::open(const std::string& path)
{
    close();  // Close any previous session

    auto demuxer = Demuxer::open(path);
    if (!demuxer)
    {
        return false;
    }

    mImpl->demuxer = std::move(demuxer);
    mPath = path;
    mIsOpen = true;
    return true;
}

void FFmpegSession::close()
{
    mImpl->demuxer.reset();
    mPath.clear();
    mIsOpen = false;
}

int FFmpegSession::bestVideoStream() const
{
    if (!mIsOpen || !mImpl->demuxer)
    {
        return -1;
    }
    return mImpl->demuxer->bestVideoStream();
}

std::optional<RawImage> FFmpegSession::extractFirstFrame()
{
    if (!mIsOpen || !mImpl->demuxer)
    {
        FFL_LOG_ERROR("extractFirstFrame: session not open");
        return std::nullopt;
    }

    const AVCodec* decoder = nullptr;
    const int streamIndex = mImpl->demuxer->bestVideoStream(&decoder);
    return decodeBestVideoFrame(*mImpl->demuxer, streamIndex, decoder);
}

std::optional<RawImage> FFmpegSession::extractFrameAt(std::int64_t timestampMs)
{
    if (!mIsOpen || !mImpl->demuxer)
    {
        FFL_LOG_ERROR("extractFrameAt: session not open");
        return std::nullopt;
    }

    if (timestampMs < 0)
    {
        FFL_LOG_ERROR("extractFrameAt: negative timestamp");
        return std::nullopt;
    }

    const AVCodec* decoder = nullptr;
    const int streamIndex = mImpl->demuxer->bestVideoStream(&decoder);
    if (streamIndex < 0 || decoder == nullptr)
    {
        FFL_LOG_ERROR("extractFrameAt: no video stream");
        return std::nullopt;
    }

    if (!mImpl->demuxer->seekToMs(streamIndex, timestampMs))
    {
        return std::nullopt;
    }

    return decodeBestVideoFrame(*mImpl->demuxer, streamIndex, decoder, timestampMs);
}

std::vector<std::optional<RawImage>> FFmpegSession::extractFramesAt(
    const std::vector<std::int64_t>& timestamps)
{
    std::vector<std::optional<RawImage>> results;
    results.reserve(timestamps.size());

    for (int64_t ts : timestamps)
    {
        results.push_back(extractFrameAt(ts));
    }

    return results;
}

int FFmpegSession::bestAudioStream() const
{
    if (!mIsOpen || !mImpl->demuxer)
    {
        return -1;
    }

    for (size_t i = 0; i < mImpl->demuxer->info().streams.size(); ++i)
    {
        if (mImpl->demuxer->info().streams[i].type == MediaType::Audio)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::optional<RawAudio> FFmpegSession::extractFirstAudioFrame()
{
    return extractFirstAudioFrame(AudioOutputOptions{});
}

std::optional<RawAudio> FFmpegSession::extractFirstAudioFrame(const AudioOutputOptions& output)
{
    if (!mIsOpen || !mImpl->demuxer)
    {
        FFL_LOG_ERROR("extractFirstAudioFrame: session not open");
        return std::nullopt;
    }

    const AVCodec* decoder = nullptr;
    int streamIndex = bestAudioStream();
    if (streamIndex >= 0)
    {
        decoder = avcodec_find_decoder(
            mImpl->demuxer->format()->streams[streamIndex]->codecpar->codec_id);
    }

    if (streamIndex < 0 || decoder == nullptr)
    {
        FFL_LOG_ERROR("extractFirstAudioFrame: no audio stream");
        return std::nullopt;
    }

    return decodeBestAudioFrame(*mImpl->demuxer, streamIndex, decoder, std::nullopt, output);
}

std::optional<RawAudio> FFmpegSession::extractAudioAt(std::int64_t timestampMs)
{
    return extractAudioAt(timestampMs, AudioOutputOptions{});
}

std::optional<RawAudio> FFmpegSession::extractAudioAt(std::int64_t timestampMs,
                                                      const AudioOutputOptions& output)
{
    if (!mIsOpen || !mImpl->demuxer)
    {
        FFL_LOG_ERROR("extractAudioAt: session not open");
        return std::nullopt;
    }

    if (timestampMs < 0)
    {
        FFL_LOG_ERROR("extractAudioAt: negative timestamp");
        return std::nullopt;
    }

    const AVCodec* decoder = nullptr;
    int streamIndex = bestAudioStream();
    if (streamIndex >= 0)
    {
        decoder = avcodec_find_decoder(
            mImpl->demuxer->format()->streams[streamIndex]->codecpar->codec_id);
    }

    if (streamIndex < 0 || decoder == nullptr)
    {
        FFL_LOG_ERROR("extractAudioAt: no audio stream");
        return std::nullopt;
    }

    if (!mImpl->demuxer->seekToMs(streamIndex, timestampMs))
    {
        return std::nullopt;
    }

    return decodeBestAudioFrame(*mImpl->demuxer, streamIndex, decoder, timestampMs, output);
}

std::vector<std::optional<RawAudio>> FFmpegSession::extractAudioAt(
    const std::vector<std::int64_t>& timestamps,
    const AudioOutputOptions& output)
{
    std::vector<std::optional<RawAudio>> results;
    results.reserve(timestamps.size());

    for (int64_t ts : timestamps)
    {
        results.push_back(extractAudioAt(ts, output));
    }

    return results;
}

std::optional<std::reference_wrapper<const MediaInfo>> FFmpegSession::info() const
{
    if (!mIsOpen || !mImpl->demuxer)
    {
        return std::nullopt;
    }
    return std::cref(mImpl->demuxer->info());
}

} // namespace ucf::utilities::ffmpeg
