#pragma once

#include <memory>
#include <string>

#include "../FFmpegInternal.h"

namespace ucf::utilities::ffmpeg::detail {

// Low-level muxer: owns an AVFormatContext and centralizes output file setup,
// stream registration, header/trailer writing and packet dispatch.
class Muxer
{
public:
    static std::unique_ptr<Muxer> create(const std::string& path,
                                         const std::string& formatName = std::string());

    Muxer(const Muxer&) = delete;
    Muxer& operator=(const Muxer&) = delete;

    int addStream(const AVCodecContext* codecCtx);
    int writeHeader();
    int writePacket(const AVPacket* packet);
    int writeTrailer();

    AVFormatContext* format() const { return mFormat.get(); }

private:
    Muxer() = default;

    FormatCtxPtr mFormat;
    std::string mPath;
};

} // namespace ucf::utilities::ffmpeg::detail
