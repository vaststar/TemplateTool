#pragma once

// Per-module export macro for ClientInfoServiceTypes.
//   CLIENT_INFO_SERVICE_TYPES_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   CLIENT_INFO_SERVICE_TYPES_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(CLIENT_INFO_SERVICE_TYPES_STATIC) && defined(CLIENT_INFO_SERVICE_TYPES_SHARED)
#  error "CLIENT_INFO_SERVICE_TYPES_STATIC and CLIENT_INFO_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(CLIENT_INFO_SERVICE_TYPES_STATIC)
#    define CLIENT_INFO_SERVICE_TYPES_API
#  elif defined(CLIENT_INFO_SERVICE_TYPES_SHARED)
#    define CLIENT_INFO_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define CLIENT_INFO_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(CLIENT_INFO_SERVICE_TYPES_STATIC)
#    define CLIENT_INFO_SERVICE_TYPES_API
#  else
#    define CLIENT_INFO_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
