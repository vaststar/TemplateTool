#pragma once

// Per-module export macro for ClientInfoServiceImpl.
//   CLIENT_INFO_SERVICE_IMPL_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   CLIENT_INFO_SERVICE_IMPL_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(CLIENT_INFO_SERVICE_IMPL_STATIC) && defined(CLIENT_INFO_SERVICE_IMPL_SHARED)
#  error "CLIENT_INFO_SERVICE_IMPL_STATIC and CLIENT_INFO_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(CLIENT_INFO_SERVICE_IMPL_STATIC)
#    define CLIENT_INFO_SERVICE_IMPL_API
#  elif defined(CLIENT_INFO_SERVICE_IMPL_SHARED)
#    define CLIENT_INFO_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define CLIENT_INFO_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(CLIENT_INFO_SERVICE_IMPL_STATIC)
#    define CLIENT_INFO_SERVICE_IMPL_API
#  else
#    define CLIENT_INFO_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
