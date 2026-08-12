#pragma once

// Per-module export macro for ContactServiceTypes.
//   CONTACT_SERVICE_TYPES_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   CONTACT_SERVICE_TYPES_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(CONTACT_SERVICE_TYPES_STATIC) && defined(CONTACT_SERVICE_TYPES_SHARED)
#  error "CONTACT_SERVICE_TYPES_STATIC and CONTACT_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(CONTACT_SERVICE_TYPES_STATIC)
#    define CONTACT_SERVICE_TYPES_API
#  elif defined(CONTACT_SERVICE_TYPES_SHARED)
#    define CONTACT_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define CONTACT_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(CONTACT_SERVICE_TYPES_STATIC)
#    define CONTACT_SERVICE_TYPES_API
#  else
#    define CONTACT_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
