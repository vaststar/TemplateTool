#pragma once

// Per-module export macro for MediaService.
//   MEDIA_SERVICE_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   MEDIA_SERVICE_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(MEDIA_SERVICE_STATIC) && defined(MEDIA_SERVICE_SHARED)
#  error "MEDIA_SERVICE_STATIC and MEDIA_SERVICE_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(MEDIA_SERVICE_STATIC)
#    define MEDIA_SERVICE_API
#  elif defined(MEDIA_SERVICE_SHARED)
#    define MEDIA_SERVICE_API __declspec(dllexport)
#  else
#    define MEDIA_SERVICE_API __declspec(dllimport)
#  endif
#else
#  if defined(MEDIA_SERVICE_STATIC)
#    define MEDIA_SERVICE_API
#  else
#    define MEDIA_SERVICE_API __attribute__((visibility("default")))
#  endif
#endif
