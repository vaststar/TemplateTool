#pragma once

// Per-module export macro for ImageProcessUtils.
//   IMAGE_PROCESS_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   IMAGE_PROCESS_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(IMAGE_PROCESS_UTILS_STATIC) && defined(IMAGE_PROCESS_UTILS_SHARED)
#  error "IMAGE_PROCESS_UTILS_STATIC and IMAGE_PROCESS_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(IMAGE_PROCESS_UTILS_STATIC)
#    define IMAGE_PROCESS_UTILS_API
#  elif defined(IMAGE_PROCESS_UTILS_SHARED)
#    define IMAGE_PROCESS_UTILS_API __declspec(dllexport)
#  else
#    define IMAGE_PROCESS_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(IMAGE_PROCESS_UTILS_STATIC)
#    define IMAGE_PROCESS_UTILS_API
#  else
#    define IMAGE_PROCESS_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
