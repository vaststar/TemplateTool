#pragma once

// Per-module export macro for ImageServiceTypes.
//   IMAGE_SERVICE_TYPES_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   IMAGE_SERVICE_TYPES_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(IMAGE_SERVICE_TYPES_STATIC) && defined(IMAGE_SERVICE_TYPES_SHARED)
#  error "IMAGE_SERVICE_TYPES_STATIC and IMAGE_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(IMAGE_SERVICE_TYPES_STATIC)
#    define IMAGE_SERVICE_TYPES_API
#  elif defined(IMAGE_SERVICE_TYPES_SHARED)
#    define IMAGE_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define IMAGE_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(IMAGE_SERVICE_TYPES_STATIC)
#    define IMAGE_SERVICE_TYPES_API
#  else
#    define IMAGE_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
