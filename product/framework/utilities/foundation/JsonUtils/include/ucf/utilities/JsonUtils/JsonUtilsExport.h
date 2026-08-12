#pragma once

// Per-module export macro for JsonUtils.
//   JSON_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   JSON_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(JSON_UTILS_STATIC) && defined(JSON_UTILS_SHARED)
#  error "JSON_UTILS_STATIC and JSON_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(JSON_UTILS_STATIC)
#    define JSON_UTILS_API
#  elif defined(JSON_UTILS_SHARED)
#    define JSON_UTILS_API __declspec(dllexport)
#  else
#    define JSON_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(JSON_UTILS_STATIC)
#    define JSON_UTILS_API
#  else
#    define JSON_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
