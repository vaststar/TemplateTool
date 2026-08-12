#pragma once

// Per-module export macro for CompressionUtils.
//   COMPRESSION_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   COMPRESSION_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(COMPRESSION_UTILS_STATIC) && defined(COMPRESSION_UTILS_SHARED)
#  error "COMPRESSION_UTILS_STATIC and COMPRESSION_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(COMPRESSION_UTILS_STATIC)
#    define COMPRESSION_UTILS_API
#  elif defined(COMPRESSION_UTILS_SHARED)
#    define COMPRESSION_UTILS_API __declspec(dllexport)
#  else
#    define COMPRESSION_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(COMPRESSION_UTILS_STATIC)
#    define COMPRESSION_UTILS_API
#  else
#    define COMPRESSION_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
