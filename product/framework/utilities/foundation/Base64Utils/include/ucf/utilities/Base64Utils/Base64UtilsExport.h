#pragma once

// Per-module export macro for Base64Utils.
//   BASE64_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   BASE64_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(BASE64_UTILS_STATIC) && defined(BASE64_UTILS_SHARED)
#  error "BASE64_UTILS_STATIC and BASE64_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(BASE64_UTILS_STATIC)
#    define BASE64_UTILS_API
#  elif defined(BASE64_UTILS_SHARED)
#    define BASE64_UTILS_API __declspec(dllexport)
#  else
#    define BASE64_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(BASE64_UTILS_STATIC)
#    define BASE64_UTILS_API
#  else
#    define BASE64_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
