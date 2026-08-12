#pragma once

#if defined(MINI_APP_SERVICE_IMPL_STATIC) && defined(MINI_APP_SERVICE_IMPL_SHARED)
#  error "MINI_APP_SERVICE_IMPL_STATIC and MINI_APP_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(MINI_APP_SERVICE_IMPL_STATIC)
#    define MINI_APP_SERVICE_IMPL_API
#  elif defined(MINI_APP_SERVICE_IMPL_SHARED)
#    define MINI_APP_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define MINI_APP_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(MINI_APP_SERVICE_IMPL_STATIC)
#    define MINI_APP_SERVICE_IMPL_API
#  else
#    define MINI_APP_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
