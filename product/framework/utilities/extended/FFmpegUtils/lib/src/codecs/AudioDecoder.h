#pragma once

#include <memory>

#include "../FFmpegInternal.h"

namespace ucf::utilities::ffmpeg::detail {

// Low-level audio decoder: owns an AVCodecContext and wraps the libav
// send-packet / receive-frame loop. Private module (libav types allowed);
// higher layers feed it packets (e.g. from Demuxer) and read decoded frames.
class AudioDecoder
{
public:
    // Creates a decoder for @p decoder configured from @p par. Returns nullptr
    // on failure (alloc / params / open).
    static std::unique_ptr<AudioDecoder> create(const AVCodec* decoder,
                                                const AVCodecParameters* par);

    AudioDecoder(const AudioDecoder&) = delete;
    AudioDecoder& operator=(const AudioDecoder&) = delete;

    // Sends a packet to the decoder; pass nullptr to flush. Returns the libav
    // result (0 on success).
    int send(const AVPacket* packet) { return avcodec_send_packet(mCtx.get(), packet); }

    // Receives the next decoded frame into @p frame. Returns 0 on success,
    // AVERROR(EAGAIN) if more input is needed, AVERROR_EOF at end.
    int receive(AVFrame* frame) { return avcodec_receive_frame(mCtx.get(), frame); }

    int sampleRate() const { return mCtx->sample_rate; }
    const AVChannelLayout* channelLayout() const { return &mCtx->ch_layout; }
    AVSampleFormat sampleFormat() const { return mCtx->sample_fmt; }
    int channels() const { return mCtx->ch_layout.nb_channels; }

private:
    AudioDecoder() = default;

    CodecCtxPtr mCtx;
};

} // namespace ucf::utilities::ffmpeg::detail
