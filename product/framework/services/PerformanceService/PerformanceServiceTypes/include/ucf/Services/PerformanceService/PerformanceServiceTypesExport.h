#pragma once

// Per-module export macro for PerformanceServiceTypes.
//   PERFORMANCE_SERVICE_TYPES_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   PERFORMANCE_SERVICE_TYPES_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(PERFORMANCE_SERVICE_TYPES_STATIC) && defined(PERFORMANCE_SERVICE_TYPES_SHARED)
#  error "PERFORMANCE_SERVICE_TYPES_STATIC and PERFORMANCE_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(PERFORMANCE_SERVICE_TYPES_STATIC)
#    define PERFORMANCE_SERVICE_TYPES_API
#  elif defined(PERFORMANCE_SERVICE_TYPES_SHARED)
#    define PERFORMANCE_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define PERFORMANCE_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(PERFORMANCE_SERVICE_TYPES_STATIC)
#    define PERFORMANCE_SERVICE_TYPES_API
#  else
#    define PERFORMANCE_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
