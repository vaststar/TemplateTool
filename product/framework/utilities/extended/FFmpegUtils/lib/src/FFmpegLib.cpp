#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegLib.h>
#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegSession.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>

extern "C" {
#include <libavutil/audio_fifo.h>
}

#include "codecs/AudioEncoder.h"
#include "codecs/AudioDecoder.h"
#include "containers/Demuxer.h"
#include "FFmpegInternal.h"
#include "containers/Muxer.h"
#include "filters/Resampler.h"
#include "filters/Scaler.h"
#include "codecs/VideoDecoder.h"

namespace ucf::utilities::ffmpeg {

using namespace detail;

namespace {

std::string formatVersion(unsigned version)
{
    return std::to_string(AV_VERSION_MAJOR(version)) + "."
         + std::to_string(AV_VERSION_MINOR(version)) + "."
         + std::to_string(AV_VERSION_MICRO(version));
}

AVSampleFormat toAVSampleFormat(SampleFormat format)
{
    switch (format)
    {
    case SampleFormat::S16: return AV_SAMPLE_FMT_S16;
    case SampleFormat::S32: return AV_SAMPLE_FMT_S32;
    case SampleFormat::FLT: return AV_SAMPLE_FMT_FLT;
    case SampleFormat::DBL: return AV_SAMPLE_FMT_DBL;
    }
    return AV_SAMPLE_FMT_S16;
}

AVCodecID toPCMCodecId(SampleFormat format)
{
    switch (format)
    {
    case SampleFormat::S16: return AV_CODEC_ID_PCM_S16LE;
    case SampleFormat::S32: return AV_CODEC_ID_PCM_S32LE;
    case SampleFormat::FLT: return AV_CODEC_ID_PCM_F32LE;
    case SampleFormat::DBL: return AV_CODEC_ID_PCM_F64LE;
    }
    return AV_CODEC_ID_PCM_S16LE;
}

bool writeAllPackets(detail::AudioEncoder& encoder,
                     detail::Muxer& muxer,
                     int streamIndex,
                     AVRational streamTimeBase);

bool writeAudioInternal(const std::string& outputPath,
                        const RawAudio& audio,
                        const AudioEncodeOptions& options);

std::string lowerCopy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string guessFormatNameFromPath(const std::string& path)
{
    const std::string ext = lowerCopy(std::filesystem::path(path).extension().string());
    if (ext == ".wav")
    {
        return "wav";
    }
    if (ext == ".m4a")
    {
        return "ipod";
    }
    if (ext == ".mp4")
    {
        return "mp4";
    }
    if (ext == ".aac")
    {
        return "adts";
    }
    return std::string();
}

bool isAacContainer(const std::string& formatName)
{
    const std::string f = lowerCopy(formatName);
    return f == "ipod" || f == "mp4" || f == "adts" || f == "aac";
}

AVSampleFormat selectEncoderSampleFormat(const AVCodec* codec,
                                         AVSampleFormat preferred,
                                         bool preferFloatPlanar)
{
    if (codec == nullptr || codec->sample_fmts == nullptr)
    {
        return preferred;
    }

    for (const AVSampleFormat* p = codec->sample_fmts; *p != AV_SAMPLE_FMT_NONE; ++p)
    {
        if (*p == preferred)
        {
            return preferred;
        }
    }

    if (preferFloatPlanar)
    {
        for (const AVSampleFormat* p = codec->sample_fmts; *p != AV_SAMPLE_FMT_NONE; ++p)
        {
            if (*p == AV_SAMPLE_FMT_FLTP)
            {
                return *p;
            }
        }
    }

    return codec->sample_fmts[0];
}

bool fillAudioFrameFromFifo(AVAudioFifo* fifo,
                            const AVCodecContext* encoderCtx,
                            int nbSamples,
                            std::int64_t pts,
                            FramePtr& frame)
{
    frame.reset(av_frame_alloc());
    if (!frame)
    {
        FFL_LOG_ERROR("writeAudio: cannot allocate frame");
        return false;
    }

    frame->nb_samples = nbSamples;
    frame->sample_rate = encoderCtx->sample_rate;
    frame->format = encoderCtx->sample_fmt;
    if (int ret = av_channel_layout_copy(&frame->ch_layout, &encoderCtx->ch_layout); ret < 0)
    {
        FFL_LOG_ERROR("writeAudio: cannot copy frame channel layout (" << errorString(ret) << ")");
        return false;
    }
    if (int ret = av_frame_get_buffer(frame.get(), 0); ret < 0)
    {
        FFL_LOG_ERROR("writeAudio: cannot allocate frame buffer (" << errorString(ret) << ")");
        return false;
    }

    if (av_audio_fifo_read(fifo, reinterpret_cast<void**>(frame->data), nbSamples) < nbSamples)
    {
        FFL_LOG_ERROR("writeAudio: cannot read from audio fifo");
        return false;
    }
    frame->pts = pts;
    return true;
}

bool encodeRawAudioBuffer(const RawAudio& audio,
                          AudioEncoder& encoder,
                          Muxer& muxer,
                          int streamIndex,
                          AVRational streamTimeBase)
{
    AVCodecContext* encCtx = encoder.context();
    if (encCtx == nullptr)
    {
        return false;
    }

    AVChannelLayout srcLayout{};
    av_channel_layout_default(&srcLayout, audio.channels);

    SwrContext* swrRaw = nullptr;
    int ret = swr_alloc_set_opts2(&swrRaw,
                                  &encCtx->ch_layout,
                                  encCtx->sample_fmt,
                                  encCtx->sample_rate,
                                  &srcLayout,
                                  toAVSampleFormat(audio.format),
                                  audio.sampleRate,
                                  0,
                                  nullptr);
    av_channel_layout_uninit(&srcLayout);
    if (ret < 0 || swrRaw == nullptr)
    {
        FFL_LOG_ERROR("writeAudio: cannot create swresample context (" << errorString(ret) << ")");
        return false;
    }
    SwrPtr swr(swrRaw);
    if (int initRet = swr_init(swr.get()); initRet < 0)
    {
        FFL_LOG_ERROR("writeAudio: cannot init swresample context (" << errorString(initRet) << ")");
        return false;
    }

    std::unique_ptr<AVAudioFifo, decltype(&av_audio_fifo_free)> fifo(
        av_audio_fifo_alloc(encCtx->sample_fmt, encCtx->ch_layout.nb_channels, 1),
        &av_audio_fifo_free);
    if (!fifo)
    {
        FFL_LOG_ERROR("writeAudio: cannot allocate audio fifo");
        return false;
    }

    const int inBytesPerFrame = bytesPerSample(audio.format) * audio.channels;
    const int totalInSamples = static_cast<int>(audio.samples.size() / static_cast<size_t>(inBytesPerFrame));
    const int inputChunk = 4096;
    std::int64_t nextPts = 0;

    auto pushConverted = [&](const uint8_t* srcPtr, int srcSamples) -> bool {
        const int maxDst = av_rescale_rnd(swr_get_delay(swr.get(), audio.sampleRate) + srcSamples,
                                          encCtx->sample_rate, audio.sampleRate, AV_ROUND_UP);
        if (maxDst <= 0)
        {
            return true;
        }

        uint8_t** dstData = nullptr;
        int dstLineSize = 0;
        if (int allocRet = av_samples_alloc_array_and_samples(&dstData,
                                                               &dstLineSize,
                                                               encCtx->ch_layout.nb_channels,
                                                               maxDst,
                                                               encCtx->sample_fmt,
                                                               0);
            allocRet < 0)
        {
            FFL_LOG_ERROR("writeAudio: cannot allocate converted sample buffer ("
                          << errorString(allocRet) << ")");
            return false;
        }

        const uint8_t* srcData[1] = {srcPtr};
        const uint8_t** srcDataPtr = srcPtr != nullptr ? srcData : nullptr;
        const int converted = swr_convert(swr.get(), dstData, maxDst, srcDataPtr, srcSamples);
        if (converted < 0)
        {
            FFL_LOG_ERROR("writeAudio: sample conversion failed (" << errorString(converted) << ")");
            av_freep(&dstData[0]);
            av_freep(&dstData);
            return false;
        }

        if (converted > 0)
        {
            if (int growRet = av_audio_fifo_realloc(fifo.get(), av_audio_fifo_size(fifo.get()) + converted);
                growRet < 0)
            {
                FFL_LOG_ERROR("writeAudio: cannot grow audio fifo (" << errorString(growRet) << ")");
                av_freep(&dstData[0]);
                av_freep(&dstData);
                return false;
            }
            if (av_audio_fifo_write(fifo.get(), reinterpret_cast<void**>(dstData), converted) < converted)
            {
                FFL_LOG_ERROR("writeAudio: cannot write converted samples to fifo");
                av_freep(&dstData[0]);
                av_freep(&dstData);
                return false;
            }
        }

        av_freep(&dstData[0]);
        av_freep(&dstData);
        return true;
    };

    auto consumeFifo = [&](bool flushTail) -> bool {
        const int frameSize = encCtx->frame_size;
        while (true)
        {
            const int available = av_audio_fifo_size(fifo.get());
            if (available <= 0)
            {
                return true;
            }

            int nbSamples = 0;
            if (frameSize > 0)
            {
                if (available < frameSize)
                {
                    if (!flushTail)
                    {
                        return true;
                    }
                    nbSamples = available;
                }
                else
                {
                    nbSamples = frameSize;
                }
            }
            else
            {
                nbSamples = available;
            }

            FramePtr frame;
            if (!fillAudioFrameFromFifo(fifo.get(), encCtx, nbSamples, nextPts, frame))
            {
                return false;
            }
            nextPts += nbSamples;

            if (int sendRet = encoder.send(frame.get()); sendRet < 0)
            {
                FFL_LOG_ERROR("writeAudio: send frame failed (" << errorString(sendRet) << ")");
                return false;
            }
            if (!writeAllPackets(encoder, muxer, streamIndex, streamTimeBase))
            {
                return false;
            }
        }
    };

    for (int offset = 0; offset < totalInSamples;)
    {
        const int chunk = std::min(inputChunk, totalInSamples - offset);
        const uint8_t* src = audio.samples.data() + static_cast<size_t>(offset * inBytesPerFrame);
        if (!pushConverted(src, chunk))
        {
            return false;
        }
        if (!consumeFifo(false))
        {
            return false;
        }
        offset += chunk;
    }

    while (true)
    {
        const int maxDst = av_rescale_rnd(swr_get_delay(swr.get(), audio.sampleRate),
                                          encCtx->sample_rate, audio.sampleRate, AV_ROUND_UP);
        if (maxDst <= 0)
        {
            break;
        }

        if (!pushConverted(nullptr, 0))
        {
            return false;
        }

        const int after = av_audio_fifo_size(fifo.get());
        if (after <= 0)
        {
            break;
        }

        if (!consumeFifo(false))
        {
            return false;
        }
        if (swr_get_delay(swr.get(), audio.sampleRate) == 0)
        {
            break;
        }
    }

    if (!consumeFifo(true))
    {
        return false;
    }

    if (int flushRet = encoder.send(nullptr); flushRet < 0)
    {
        FFL_LOG_ERROR("writeAudio: flush encoder failed (" << errorString(flushRet) << ")");
        return false;
    }
    return writeAllPackets(encoder, muxer, streamIndex, streamTimeBase);
}

bool writeAllPackets(detail::AudioEncoder& encoder,
                     detail::Muxer& muxer,
                     int streamIndex,
                     AVRational streamTimeBase)
{
    PacketPtr packet(av_packet_alloc());
    if (!packet)
    {
        FFL_LOG_ERROR("writeWav: cannot allocate packet");
        return false;
    }

    while (true)
    {
        int ret = encoder.receive(packet.get());
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return true;
        }
        if (ret < 0)
        {
            FFL_LOG_ERROR("writeWav: receive encoded packet failed (" << errorString(ret) << ")");
            return false;
        }

        packet->stream_index = streamIndex;
        av_packet_rescale_ts(packet.get(), encoder.context()->time_base, streamTimeBase);
        if (int writeRet = muxer.writePacket(packet.get()); writeRet < 0)
        {
            FFL_LOG_ERROR("writeWav: write packet failed (" << errorString(writeRet) << ")");
            return false;
        }
        av_packet_unref(packet.get());
    }
}

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
                            RawAudio& out);

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
            FFL_LOG_ERROR("decodeBestAudioFrame: cannot copy channel layout ("
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
        FFL_LOG_ERROR("decodeBestAudioFrame: invalid channel layout");
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
            FFL_LOG_ERROR("decodeBestVideoFrame: decode error: " << errorString(ret));
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
            FFL_LOG_ERROR("decodeBestAudioFrame: decode error: " << errorString(ret));
            return false;
        }
        return false;
    }
}

std::optional<RawImage> decodeBestVideoFrame(Demuxer& demuxer, int streamIndex,
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


FFmpegLib::Version FFmpegLib::version()
{
    Version v;
    v.build = av_version_info();
    v.avcodec = formatVersion(avcodec_version());
    v.avformat = formatVersion(avformat_version());
    v.avutil = formatVersion(avutil_version());
    return v;
}

std::optional<MediaInfo> FFmpegLib::probe(const std::string& url)
{
    FFmpegSession session;
    if (!session.open(url))
    {
        return std::nullopt;
    }
    auto info = session.info();
    if (!info)
    {
        return std::nullopt;
    }
    return info->get();
}

std::optional<RawImage> FFmpegLib::decodeFirstFrame(const std::string& path)
{
    FFmpegSession session;
    if (!session.open(path))
    {
        return std::nullopt;
    }
    return session.extractFirstFrame();
}

std::optional<RawAudio> FFmpegLib::decodeFirstAudioFrame(const std::string& path)
{
    return decodeFirstAudioFrame(path, AudioOutputOptions{});
}

std::optional<RawAudio> FFmpegLib::decodeFirstAudioFrame(const std::string& path,
                                                         const AudioOutputOptions& output)
{
    FFmpegSession session;
    if (!session.open(path))
    {
        return std::nullopt;
    }
    return session.extractFirstAudioFrame(output);
}

std::optional<RawAudio> FFmpegLib::extractAudioAt(const std::string& path,
                                                  std::int64_t timestampMs)
{
    return extractAudioAt(path, timestampMs, AudioOutputOptions{});
}

std::optional<RawAudio> FFmpegLib::extractAudioAt(const std::string& path,
                                                  std::int64_t timestampMs,
                                                  const AudioOutputOptions& output)
{
    if (timestampMs < 0)
    {
        FFL_LOG_ERROR("extractAudioAt: negative timestamp");
        return std::nullopt;
    }

    FFmpegSession session;
    if (!session.open(path))
    {
        return std::nullopt;
    }
    return session.extractAudioAt(timestampMs, output);
}

std::optional<RawImage> FFmpegLib::extractFrameAt(const std::string& path,
                                                  std::int64_t timestampMs)
{
    if (timestampMs < 0)
    {
        FFL_LOG_ERROR("extractFrameAt: negative timestamp");
        return std::nullopt;
    }

    FFmpegSession session;
    if (!session.open(path))
    {
        return std::nullopt;
    }
    return session.extractFrameAt(timestampMs);
}

bool FFmpegLib::writeWav(const std::string& outputPath, const RawAudio& audio)
{
    AudioEncodeOptions options;
    options.formatName = "wav";
    return writeAudioInternal(outputPath, audio, options);
}

bool FFmpegLib::writeAac(const std::string& outputPath, const RawAudio& audio)
{
    AudioEncodeOptions options;
    options.formatName = "adts";
    options.codecName = "aac";
    return writeAudioInternal(outputPath, audio, options);
}

namespace {

bool writeAudioInternal(const std::string& outputPath,
                        const RawAudio& audio,
                        const AudioEncodeOptions& options)
{
    if (outputPath.empty())
    {
        FFL_LOG_ERROR("writeAudio: empty output path");
        return false;
    }
    if (audio.sampleRate <= 0 || audio.channels <= 0)
    {
        FFL_LOG_ERROR("writeAudio: invalid sample rate/channels");
        return false;
    }

    const int bps = bytesPerSample(audio.format);
    if (bps <= 0)
    {
        FFL_LOG_ERROR("writeAudio: unsupported input sample format");
        return false;
    }
    const int bytesPerFrame = bps * audio.channels;
    if (bytesPerFrame <= 0 || (audio.samples.size() % static_cast<size_t>(bytesPerFrame)) != 0)
    {
        FFL_LOG_ERROR("writeAudio: sample buffer size is not frame-aligned");
        return false;
    }
    if (audio.samples.empty())
    {
        FFL_LOG_ERROR("writeAudio: empty sample buffer");
        return false;
    }

    std::string formatName = options.formatName;
    if (formatName.empty())
    {
        formatName = guessFormatNameFromPath(outputPath);
    }

    auto muxer = Muxer::create(outputPath, formatName);
    if (!muxer)
    {
        return false;
    }

    const AVCodec* codec = nullptr;
    if (!options.codecName.empty())
    {
        codec = avcodec_find_encoder_by_name(options.codecName.c_str());
        if (!codec)
        {
            FFL_LOG_ERROR("writeAudio: cannot find encoder by name: " << options.codecName);
            return false;
        }
    }
    else
    {
        if (isAacContainer(formatName))
        {
            codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        }
        else
        {
            codec = avcodec_find_encoder(toPCMCodecId(audio.format));
        }
        if (!codec)
        {
            FFL_LOG_ERROR("writeAudio: cannot find default encoder");
            return false;
        }
    }

    const int outRate = options.sampleRate > 0 ? options.sampleRate : audio.sampleRate;
    const int outChannels = options.channels > 0 ? options.channels : audio.channels;

    AVChannelLayout outLayout{};
    av_channel_layout_default(&outLayout, outChannels);

    const bool preferFloatPlanar = codec->id == AV_CODEC_ID_AAC;
    const AVSampleFormat inFmt = toAVSampleFormat(audio.format);
    const AVSampleFormat encFmt = selectEncoderSampleFormat(codec, inFmt, preferFloatPlanar);
    const bool useGlobalHeader = (muxer->format()->oformat->flags & AVFMT_GLOBALHEADER) != 0;

    const int bitRate = options.bitrate > 0 ? options.bitrate : 0;
    auto encoder = AudioEncoder::create(codec, outLayout, outRate, encFmt, bitRate, useGlobalHeader);
    av_channel_layout_uninit(&outLayout);
    if (!encoder)
    {
        return false;
    }
    encoder->context()->time_base = AVRational{1, outRate};

    if (int ret = muxer->addStream(encoder->context()); ret < 0)
    {
        FFL_LOG_ERROR("writeAudio: add stream failed (" << errorString(ret) << ")");
        return false;
    }
    if (int ret = muxer->writeHeader(); ret < 0)
    {
        FFL_LOG_ERROR("writeAudio: write header failed (" << errorString(ret) << ")");
        return false;
    }

    AVStream* stream = muxer->format()->streams[0];
    if (!encodeRawAudioBuffer(audio, *encoder, *muxer, 0, stream->time_base))
    {
        return false;
    }

    if (int ret = muxer->writeTrailer(); ret < 0)
    {
        FFL_LOG_ERROR("writeAudio: write trailer failed (" << errorString(ret) << ")");
        return false;
    }
    return true;
}

} // namespace

} // namespace ucf::utilities::ffmpeg
