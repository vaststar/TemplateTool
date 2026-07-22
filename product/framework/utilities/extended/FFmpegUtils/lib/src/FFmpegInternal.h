#pragma once

// =============================================================================
// FFmpegInternal - the single place inside FFmpegLibUtils that touches libav.
//
// Public headers under include/.../Lib/ must NOT include any libav header. This
// private header centralizes:
//   - the extern "C" libav includes,
//   - RAII wrappers for libav objects (no manual free anywhere else),
//   - error-code translation (av_strerror -> std::string),
//   - the av_log -> FFmpegLibLogger bridge.
// Every .cpp in this module includes this instead of libav directly.
// =============================================================================

#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "FFmpegLibLogger.h"

namespace ucf::utilities::ffmpeg::detail {

// ---- RAII deleters ---------------------------------------------------------
struct FormatCtxDeleter
{
    void operator()(AVFormatContext* c) const { if (c) avformat_close_input(&c); }
};
struct CodecCtxDeleter
{
    void operator()(AVCodecContext* c) const { if (c) avcodec_free_context(&c); }
};
struct FrameDeleter
{
    void operator()(AVFrame* f) const { if (f) av_frame_free(&f); }
};
struct PacketDeleter
{
    void operator()(AVPacket* p) const { if (p) av_packet_free(&p); }
};
struct SwsDeleter
{
    void operator()(SwsContext* s) const { if (s) sws_freeContext(s); }
};
struct SwrDeleter
{
    void operator()(SwrContext* s) const { if (s) swr_free(&s); }
};

using FormatCtxPtr = std::unique_ptr<AVFormatContext, FormatCtxDeleter>;
using CodecCtxPtr = std::unique_ptr<AVCodecContext, CodecCtxDeleter>;
using FramePtr = std::unique_ptr<AVFrame, FrameDeleter>;
using PacketPtr = std::unique_ptr<AVPacket, PacketDeleter>;
using SwsPtr = std::unique_ptr<SwsContext, SwsDeleter>;
using SwrPtr = std::unique_ptr<SwrContext, SwrDeleter>;

// ---- Error translation -----------------------------------------------------
inline std::string errorString(int averror)
{
    char buf[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(averror, buf, sizeof(buf));
    return buf;
}

// ---- av_log bridge ---------------------------------------------------------
// Routes libav's internal logging into FFmpegLibLogger instead of stderr, so
// noisy diagnostics (e.g. "Invalid PNG signature") follow our logging config.
// Idempotent: installs the callback once on first use.
inline void installLogBridge()
{
    static const bool installed = [] {
        av_log_set_callback([](void* avcl, int level, const char* fmt, va_list vl) {
            if (level > av_log_get_level())
            {
                return;
            }
            char line[1024] = {};
            int printPrefix = 1;
            av_log_format_line(avcl, level, fmt, vl, line, sizeof(line), &printPrefix);

            // Trim the trailing newline libav appends.
            std::string msg(line);
            while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r'))
            {
                msg.pop_back();
            }
            if (msg.empty())
            {
                return;
            }

            if (level <= AV_LOG_ERROR)       { FFL_LOG_ERROR(msg); }
            else if (level <= AV_LOG_WARNING) { FFL_LOG_WARN(msg); }
            else if (level <= AV_LOG_INFO)    { FFL_LOG_INFO(msg); }
            else                              { FFL_LOG_DEBUG(msg); }
        });
        return true;
    }();
    (void)installed;
}

} // namespace ucf::utilities::ffmpeg::detail
