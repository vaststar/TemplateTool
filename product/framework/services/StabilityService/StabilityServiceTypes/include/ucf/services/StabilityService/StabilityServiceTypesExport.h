#pragma once

// Per-module export macro for StabilityServiceTypes.
//   STABILITY_SERVICE_TYPES_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   STABILITY_SERVICE_TYPES_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(STABILITY_SERVICE_TYPES_STATIC) && defined(STABILITY_SERVICE_TYPES_SHARED)
#  error "STABILITY_SERVICE_TYPES_STATIC and STABILITY_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(STABILITY_SERVICE_TYPES_STATIC)
#    define STABILITY_SERVICE_TYPES_API
#  elif defined(STABILITY_SERVICE_TYPES_SHARED)
#    define STABILITY_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define STABILITY_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(STABILITY_SERVICE_TYPES_STATIC)
#    define STABILITY_SERVICE_TYPES_API
#  else
#    define STABILITY_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
