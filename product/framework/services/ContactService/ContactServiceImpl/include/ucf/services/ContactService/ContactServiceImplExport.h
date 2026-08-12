#pragma once

// Per-module export macro for ContactServiceImpl.
//   CONTACT_SERVICE_IMPL_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   CONTACT_SERVICE_IMPL_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(CONTACT_SERVICE_IMPL_STATIC) && defined(CONTACT_SERVICE_IMPL_SHARED)
#  error "CONTACT_SERVICE_IMPL_STATIC and CONTACT_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(CONTACT_SERVICE_IMPL_STATIC)
#    define CONTACT_SERVICE_IMPL_API
#  elif defined(CONTACT_SERVICE_IMPL_SHARED)
#    define CONTACT_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define CONTACT_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(CONTACT_SERVICE_IMPL_STATIC)
#    define CONTACT_SERVICE_IMPL_API
#  else
#    define CONTACT_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
