#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegTypes.h>

#include "../FFmpegInternal.h"

namespace ucf::utilities::ffmpeg::detail {

// Low-level container reader: owns an AVFormatContext and centralizes "open +
// probe streams + build MediaInfo + read packets". This is a private module
// (libav types are allowed here); the public facade composes it. Higher-level
// modules (decoders/encoders) read packets from it.
class Demuxer
{
public:
    // Opens @p url and probes stream info. Returns nullptr on any failure
    // (cannot open, no stream info). Installs the av_log bridge.
    static std::unique_ptr<Demuxer> open(const std::string& url);

    Demuxer(const Demuxer&) = delete;
    Demuxer& operator=(const Demuxer&) = delete;

    // Neutral, libav-free metadata built once at open().
    const MediaInfo& info() const { return mInfo; }

    // Underlying context, for modules that need stream parameters.
    AVFormatContext* format() const { return mFormat.get(); }

    // Seeks the selected stream to @p timestampMs. Returns false on failure.
    bool seekToMs(int streamIndex, std::int64_t timestampMs) const;

    // Index of the best video stream, or <0 if none. When found, @p outDecoder
    // (optional) receives the matching decoder.
    int bestVideoStream(const AVCodec** outDecoder = nullptr) const;

    // Reads the next packet into @p pkt. Returns the av_read_frame result
    // (>= 0 on success, < 0 on error/EOF).
    int readPacket(AVPacket* pkt) { return av_read_frame(mFormat.get(), pkt); }

private:
    Demuxer() = default;

    FormatCtxPtr mFormat;
    MediaInfo mInfo;
};

} // namespace ucf::utilities::ffmpeg::detail
