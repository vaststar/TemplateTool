#pragma once

// Per-module export macro for OSUtils.
//   OS_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   OS_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(OS_UTILS_STATIC) && defined(OS_UTILS_SHARED)
#  error "OS_UTILS_STATIC and OS_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(OS_UTILS_STATIC)
#    define OS_UTILS_API
#  elif defined(OS_UTILS_SHARED)
#    define OS_UTILS_API __declspec(dllexport)
#  else
#    define OS_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(OS_UTILS_STATIC)
#    define OS_UTILS_API
#  else
#    define OS_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
