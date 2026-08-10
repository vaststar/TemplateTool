#pragma once

// Per-module export macro for FFmpegLibUtils.
//   FFMPEG_LIB_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   FFMPEG_LIB_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(FFMPEG_LIB_UTILS_STATIC) && defined(FFMPEG_LIB_UTILS_SHARED)
#  error "FFMPEG_LIB_UTILS_STATIC and FFMPEG_LIB_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(FFMPEG_LIB_UTILS_STATIC)
#    define FFmpegLibUtils_EXPORT
#  elif defined(FFMPEG_LIB_UTILS_SHARED)
#    define FFmpegLibUtils_EXPORT __declspec(dllexport)
#  else
#    define FFmpegLibUtils_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(FFMPEG_LIB_UTILS_STATIC)
#    define FFmpegLibUtils_EXPORT
#  else
#    define FFmpegLibUtils_EXPORT __attribute__((visibility("default")))
#  endif
#endif
