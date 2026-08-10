#pragma once

// Per-module export macro for ThreadPoolUtils.
//   THREAD_POOL_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   THREAD_POOL_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(THREAD_POOL_UTILS_STATIC) && defined(THREAD_POOL_UTILS_SHARED)
#  error "THREAD_POOL_UTILS_STATIC and THREAD_POOL_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(THREAD_POOL_UTILS_STATIC)
#    define THREAD_POOL_UTILS_API
#  elif defined(THREAD_POOL_UTILS_SHARED)
#    define THREAD_POOL_UTILS_API __declspec(dllexport)
#  else
#    define THREAD_POOL_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(THREAD_POOL_UTILS_STATIC)
#    define THREAD_POOL_UTILS_API
#  else
#    define THREAD_POOL_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
