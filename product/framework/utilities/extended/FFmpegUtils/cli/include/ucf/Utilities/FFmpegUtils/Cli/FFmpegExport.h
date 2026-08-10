#pragma once

// Per-module export macro for FFmpegCliUtils.
//   FFMPEG_CLI_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   FFMPEG_CLI_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(FFMPEG_CLI_UTILS_STATIC) && defined(FFMPEG_CLI_UTILS_SHARED)
#  error "FFMPEG_CLI_UTILS_STATIC and FFMPEG_CLI_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(FFMPEG_CLI_UTILS_STATIC)
#    define FFmpegUtils_EXPORT
#  elif defined(FFMPEG_CLI_UTILS_SHARED)
#    define FFmpegUtils_EXPORT __declspec(dllexport)
#  else
#    define FFmpegUtils_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(FFMPEG_CLI_UTILS_STATIC)
#    define FFmpegUtils_EXPORT
#  else
#    define FFmpegUtils_EXPORT __attribute__((visibility("default")))
#  endif
#endif
