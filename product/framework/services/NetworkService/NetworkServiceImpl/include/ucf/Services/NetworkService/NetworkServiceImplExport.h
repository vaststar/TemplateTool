#pragma once

// Per-module export macro for NetworkServiceImpl.
//   NETWORK_SERVICE_IMPL_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   NETWORK_SERVICE_IMPL_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(NETWORK_SERVICE_IMPL_STATIC) && defined(NETWORK_SERVICE_IMPL_SHARED)
#  error "NETWORK_SERVICE_IMPL_STATIC and NETWORK_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(NETWORK_SERVICE_IMPL_STATIC)
#    define NETWORK_SERVICE_IMPL_API
#  elif defined(NETWORK_SERVICE_IMPL_SHARED)
#    define NETWORK_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define NETWORK_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(NETWORK_SERVICE_IMPL_STATIC)
#    define NETWORK_SERVICE_IMPL_API
#  else
#    define NETWORK_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
