#pragma once

// Per-module export macro for UUIDUtils.
//   UUID_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   UUID_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(UUID_UTILS_STATIC) && defined(UUID_UTILS_SHARED)
#  error "UUID_UTILS_STATIC and UUID_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(UUID_UTILS_STATIC)
#    define UUID_UTILS_API
#  elif defined(UUID_UTILS_SHARED)
#    define UUID_UTILS_API __declspec(dllexport)
#  else
#    define UUID_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(UUID_UTILS_STATIC)
#    define UUID_UTILS_API
#  else
#    define UUID_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
