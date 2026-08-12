#pragma once

// Per-module export macro for SystemUtils.
//   SYSTEM_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   SYSTEM_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(SYSTEM_UTILS_STATIC) && defined(SYSTEM_UTILS_SHARED)
#  error "SYSTEM_UTILS_STATIC and SYSTEM_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(SYSTEM_UTILS_STATIC)
#    define SYSTEM_UTILS_API
#  elif defined(SYSTEM_UTILS_SHARED)
#    define SYSTEM_UTILS_API __declspec(dllexport)
#  else
#    define SYSTEM_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(SYSTEM_UTILS_STATIC)
#    define SYSTEM_UTILS_API
#  else
#    define SYSTEM_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
