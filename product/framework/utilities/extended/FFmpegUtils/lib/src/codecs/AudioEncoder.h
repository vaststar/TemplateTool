#pragma once

#include <memory>

#include "../FFmpegInternal.h"

namespace ucf::utilities::ffmpeg::detail {

// Low-level audio encoder: owns an AVCodecContext and wraps the libav
// send-frame / receive-packet loop. Private module (libav types allowed);
// higher layers can use it to turn interleaved PCM into encoded packets.
class AudioEncoder
{
public:
    static std::unique_ptr<AudioEncoder> create(const AVCodec* encoder,
                                                const AVChannelLayout& layout,
                                                int sampleRate,
                                                AVSampleFormat sampleFormat,
                                                int bitRate = 0,
                                                bool useGlobalHeader = false);

    AudioEncoder(const AudioEncoder&) = delete;
    AudioEncoder& operator=(const AudioEncoder&) = delete;

    int send(const AVFrame* frame) { return avcodec_send_frame(mCtx.get(), frame); }
    int receive(AVPacket* packet) { return avcodec_receive_packet(mCtx.get(), packet); }

    AVCodecContext* context() const { return mCtx.get(); }

private:
    AudioEncoder() = default;

    CodecCtxPtr mCtx;
};

} // namespace ucf::utilities::ffmpeg::detail
