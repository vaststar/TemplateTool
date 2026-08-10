#pragma once

// Per-module export macro for TimeUtils.
//   TIME_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   TIME_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(TIME_UTILS_STATIC) && defined(TIME_UTILS_SHARED)
#  error "TIME_UTILS_STATIC and TIME_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(TIME_UTILS_STATIC)
#    define TIME_UTILS_API
#  elif defined(TIME_UTILS_SHARED)
#    define TIME_UTILS_API __declspec(dllexport)
#  else
#    define TIME_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(TIME_UTILS_STATIC)
#    define TIME_UTILS_API
#  else
#    define TIME_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
