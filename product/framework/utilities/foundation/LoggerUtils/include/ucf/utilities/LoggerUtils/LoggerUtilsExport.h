#pragma once

// Per-module export macro for LoggerUtils.
//   LOGGER_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   LOGGER_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(LOGGER_UTILS_STATIC) && defined(LOGGER_UTILS_SHARED)
#  error "LOGGER_UTILS_STATIC and LOGGER_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(LOGGER_UTILS_STATIC)
#    define LOGGER_UTILS_API
#  elif defined(LOGGER_UTILS_SHARED)
#    define LOGGER_UTILS_API __declspec(dllexport)
#  else
#    define LOGGER_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(LOGGER_UTILS_STATIC)
#    define LOGGER_UTILS_API
#  else
#    define LOGGER_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
