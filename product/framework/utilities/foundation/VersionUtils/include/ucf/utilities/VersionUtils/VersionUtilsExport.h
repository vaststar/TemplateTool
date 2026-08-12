#pragma once

// Per-module export macro for VersionUtils.
//   VERSION_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   VERSION_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(VERSION_UTILS_STATIC) && defined(VERSION_UTILS_SHARED)
#  error "VERSION_UTILS_STATIC and VERSION_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(VERSION_UTILS_STATIC)
#    define VERSION_UTILS_API
#  elif defined(VERSION_UTILS_SHARED)
#    define VERSION_UTILS_API __declspec(dllexport)
#  else
#    define VERSION_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(VERSION_UTILS_STATIC)
#    define VERSION_UTILS_API
#  else
#    define VERSION_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
