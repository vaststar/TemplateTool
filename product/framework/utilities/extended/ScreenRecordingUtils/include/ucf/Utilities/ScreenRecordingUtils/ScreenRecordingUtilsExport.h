#pragma once

// Per-module export macro for ScreenRecordingUtils.
//   SCREEN_RECORDING_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   SCREEN_RECORDING_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(SCREEN_RECORDING_UTILS_STATIC) && defined(SCREEN_RECORDING_UTILS_SHARED)
#  error "SCREEN_RECORDING_UTILS_STATIC and SCREEN_RECORDING_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(SCREEN_RECORDING_UTILS_STATIC)
#    define SCREEN_RECORDING_UTILS_API
#  elif defined(SCREEN_RECORDING_UTILS_SHARED)
#    define SCREEN_RECORDING_UTILS_API __declspec(dllexport)
#  else
#    define SCREEN_RECORDING_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(SCREEN_RECORDING_UTILS_STATIC)
#    define SCREEN_RECORDING_UTILS_API
#  else
#    define SCREEN_RECORDING_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
